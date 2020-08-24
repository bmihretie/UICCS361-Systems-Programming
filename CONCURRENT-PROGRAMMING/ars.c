// a toy Airline Reservations System

#include "ars.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

struct flight_info {
	
  int next_tid;            // +1 everytime
  int nr_booked;           // booked <= seats
  pthread_mutex_t lock;    // for fine grained lock
  pthread_cond_t cond;     // condition variable per flight
  struct ticket tickets[]; // all issued tickets of this flight
	
};

//intialise the lock and condition variable
pthread_mutex_t lock =  PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int __nr_flights = 0;
int __nr_seats = 0;
struct flight_info ** flights = NULL;


static int ticket_cmp(const void * p1, const void * p2)
{
  const uint64_t v1 = *(const uint64_t *)p1;
  const uint64_t v2 = *(const uint64_t *)p2;
  if (v1 < v2)
    return -1;
  else if (v1 > v2)
    return 1;
  else
    return 0;
}

void tickets_sort(struct ticket * ts, int n)
{
  qsort(ts, n, sizeof(*ts), ticket_cmp);
}

void ars_init(int nr_flights, int nr_seats_per_flight)
{
  flights = malloc(sizeof(*flights) * nr_flights);
  for (int i = 0; i < nr_flights; i++) {
    flights[i] = calloc(1, sizeof(flights[i][0]) + (sizeof(struct ticket) * nr_seats_per_flight));
    flights[i]->next_tid = 1;
  }
  __nr_flights = nr_flights;
  __nr_seats = nr_seats_per_flight;

}

// function that uses mutlithread and books the flight if the flight numer is valid and 
// there is available seat
int book_flight(short user_id, short flight_number)
{ 
    
  // wrong number
  if (flight_number >= __nr_flights)
	  return -1;
  
  struct flight_info * fi = flights[flight_number];
  // lock individual flights
  pthread_mutex_lock(&fi->lock);
	
  // if the seat is full unlock & return
  if (fi->nr_booked >= __nr_seats) {
	   pthread_mutex_unlock(&fi->lock); // release the lock
       return -1;
  }
	
  // if all condtions satisfies book the flight
  int tid = fi->next_tid++;
 
 
  fi->tickets[fi->nr_booked].uid = user_id;
  fi->tickets[fi->nr_booked].fid = flight_number;
  fi->tickets[fi->nr_booked].tid = tid;
 
  fi->nr_booked++;

  pthread_mutex_unlock(&fi->lock); // release the lock
  return tid;
	
} // end of book_flight()


// a helper function for cancel/change
// search a ticket of a flight and return its offset if found
static int search_ticket(struct flight_info * fi, short user_id, int ticket_number)
{
  for (int i = 0; i < fi->nr_booked; i++)
    if (fi->tickets[i].uid == user_id && fi->tickets[i].tid == ticket_number){
      return i; // cancelled
	 
	}
  return -1;
}


// a function that uses  mutlithread to cancel the flight, if the flight is valid and there is
// available seat
bool cancel_flight(short user_id, short flight_number, int ticket_number)
{
  // wrong number
  if (flight_number >= __nr_flights)
	   return false;

  struct flight_info * fi = flights[flight_number];
  // lock individual flights
  pthread_mutex_lock(&fi->lock);
   
  int offset = search_ticket(fi, user_id, ticket_number);
  // if search ticket is valid
  if (offset >= 0) {
	  
	fi->tickets[offset] = fi->tickets[fi->nr_booked-1];
	fi->nr_booked--;

	// send wake up siginal to condtion wait in book flight wait
	pthread_cond_signal(&fi->cond);

	pthread_mutex_unlock(&fi->lock); // release the lock
	return true; // cancelled
  }
	
  pthread_mutex_unlock(&fi->lock);
  return false; // not found
	
} // end of cancel_flight()


// change flight function that books new flight if the flight is cancelled successfully and
// the new flight is valid and there is available seat
int change_flight(short user_id, short old_flight_number, int old_ticket_number,
                  short new_flight_number)
{

  // wrong number or no-op
  if (old_flight_number >= __nr_flights ||
      new_flight_number >= __nr_flights ||
      old_flight_number == new_flight_number){
      return -1;
	  
  }
	  
  // two things must be done atomically: (1) cancel the old ticket and (2) book a new ticket
  // if any of the two operations cannot be done, nothing should happen
  // for example, if the new flight has no seat, the existing ticket must remain valid
  // if the old ticket number is invalid, don't acquire a new ticket
 
  struct flight_info * fi = flights[old_flight_number];
  struct flight_info * fi1 = flights[new_flight_number];
	
  // a condtion to handle race or deadlock condtion using smaller value to lock first as arule
  if(old_flight_number < new_flight_number){
	 
	 pthread_mutex_lock(&fi->lock);
     pthread_mutex_lock(&fi1->lock);
	  
  }
	 
  else{
	  pthread_mutex_lock(&fi1->lock);
      pthread_mutex_lock(&fi->lock);
	  
  }
  
	
  // check if the new flight full and return by unlocking the lock
  if (fi1->nr_booked >= __nr_seats) {
	  
	   pthread_mutex_unlock(&fi1->lock); // release the lock
       pthread_mutex_unlock(&fi->lock);  // release the lock
	   return -1;
  }
  
  // search and check if the offset of the ticket is valid
  int offset = search_ticket(fi, user_id, old_ticket_number);
   
  if(offset >= 0){
	    
	    // when condtions satisfies cance the flight
	    fi->tickets[offset] = fi->tickets[fi->nr_booked-1];
		fi->nr_booked--;
	  
	    // if old flight is sucessfully canceled and the new flight number is valid and there is 
	    // available seat, book the new flight 
	    
		int tid = fi1->next_tid++;
	 
		fi1->tickets[fi1->nr_booked].uid = user_id;
		fi1->tickets[fi1->nr_booked].fid = new_flight_number;
		fi1->tickets[fi1->nr_booked].tid = tid;
		fi1->nr_booked++;

		
	    pthread_mutex_unlock(&fi1->lock); // release the lock
        pthread_mutex_unlock(&fi->lock); // release the lock
		return tid; // return the ticket 
	
   }
	
  
  pthread_mutex_unlock(&fi1->lock);
  pthread_mutex_unlock(&fi->lock);
  return -1;

} // end of change_flight()


// malloc and dump all tickets in the returned array
struct ticket * dump_tickets(int * n_out)
{
  int n = 0;
  for (int i = 0; i < __nr_flights; i++)
    n += flights[i]->nr_booked;

  struct ticket * const buf = malloc(sizeof(*buf) * n);
  assert(buf);
  n = 0;
  for (int i = 0; i < __nr_flights; i++) {
    memcpy(buf+n, flights[i]->tickets, sizeof(*buf) * flights[i]->nr_booked);
    n += flights[i]->nr_booked;
  }
  *n_out = n; // number of tickets
  return buf;
}

// the function that uses condition variables to wait and book if the flight is booked
//  or the seat is full
int book_flight_can_wait(short user_id, short flight_number)
{
 
  // if it is wrong number
  if (flight_number >= __nr_flights)
	  return -1;
 
  struct flight_info * fi2 = flights[flight_number];
  pthread_mutex_lock(&fi2->lock); // lock each flight
	
  //if flight is full loop unttil the seat is available
  while(fi2->nr_booked >= __nr_seats) 
	  pthread_cond_wait(&fi2->cond,&fi2->lock);//condtion to sleep and wait
  
     // if the seat is available during wake up call from cancel flight
     // go a head and book the flight 
 
	  int tid = fi2->next_tid++;

	  fi2->tickets[fi2->nr_booked].uid = user_id;
	  fi2->tickets[fi2->nr_booked].fid = flight_number;
	  fi2->tickets[fi2->nr_booked].tid = tid;

	  fi2->nr_booked++;

	  pthread_mutex_unlock(&fi2->lock); // release the lock
	  return tid; // return ticket if sucessful
 
} // end of book_flight_can_wait()
