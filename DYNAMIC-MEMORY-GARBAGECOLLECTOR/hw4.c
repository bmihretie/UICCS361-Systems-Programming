#include "memlib.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define WSIZE 4 /* Word and header/footer size (bytes) */
#define HDRP(bp) ((char *)(bp)-WSIZE) /* Given block ptr bp, compute address of next and previous blocks */
#include "mm.h"
#include "hw4.h"

struct node * root_table[ROOT_NR] = {};

int in_use(unsigned int * hdrp) {
  return (*hdrp) & 0x1u;
}

// the three lowest bits are not part of the size
// 0x1 is already used for free blocks; we use 0x2
#define MARK_BIT (((unsigned int)0x2))

// marking related operations
int is_marked(unsigned int * hdrp) {
  return ((*hdrp) & MARK_BIT) >> 1; // return 1 or 0
}

void mark(unsigned int * hdrp) {
  (*hdrp) |= MARK_BIT;
}

void unmark(unsigned int * hdrp) {
  (*hdrp) &= ~MARK_BIT;
}

// same to GET_SIZE in mm.c
unsigned int block_size(unsigned int * hdrp) {
  return (*hdrp) & (~7u);
}

void* next_hdrp(unsigned int * hdrp) {
  const unsigned int size = block_size(hdrp);
  if (size == 0) {
    fprintf(stderr,"%s Panic, chunk is of zero size.\n", __func__);
    exit(-1);
  }
  hdrp = (unsigned int *)(((char *)hdrp) + size);
  return block_size(hdrp) ? hdrp : NULL;
}

void heap_stat(const char * msg)
{
  void *hdrp = mm_first_hdr();
  size_t nr_inuse = 0;
  size_t sz_inuse = 0;
  size_t nr_free = 0;
  size_t sz_free = 0;

  while (hdrp && block_size(hdrp)) {
    if (in_use(hdrp)) {
      nr_inuse++;
      sz_inuse += block_size(hdrp);
    } else {
      nr_free++;
      sz_free += block_size(hdrp);
    }
    if (is_marked(hdrp))
      printf("%s WARNING: found a mark\n", __func__);
    hdrp = next_hdrp(hdrp);
  }

  printf("%s: %s: heapsize %zu  inuse %zu %zu  free %zu %zu\n", __func__, msg,
      mem_heapsize(), nr_inuse, sz_inuse, nr_free, sz_free);
}


// helper function for gc that traverse the linked lists from the root table
// and mark all the reachable nodes. 
void _mark_helper(){
	
	// traverse the root nodes
	for (int i = 0; i < ROOT_NR; i++){

		
       //assign the root node to temp not to override it and use it again
       struct node* temp = root_table[i];
		 
       // trversing the children nodes	
       while (temp != NULL){

	       if(!is_marked((unsigned int*)HDRP(temp))){
		       mark((unsigned int*)HDRP(temp));
	       }
	       
	       // update the temp
	       struct node * next = temp->next;
	       temp = next;

       } // end of while loop

  } // end of the outer for loop
	
	
} // end of _mark_helper()


// helper function that traverse the list of all memory blocks and free the garbage 
// blocks (those inuse but without a mark). And clear the marks for the live blocks. 
void _sweep_helper(){
	
	// address of the first heap header
	void *heapHeaderAdress = mm_first_hdr();
	
	// loop until the end of heapHeaderAdress
	while(heapHeaderAdress){
		
		//if the block is not marked and not in use free it
		if(!is_marked(heapHeaderAdress ) && in_use(heapHeaderAdress)){
		  // calling mm_free to free the block and coalesce
			mm_free(heapHeaderAdress + WSIZE);
		
		 }
		
		 else{ //if the block is marked, unmark it
			 
			 unmark(heapHeaderAdress );
			
		 }
		
		 // update the adress of heap headr by using next_hdrp
		 heapHeaderAdress = (next_hdrp(heapHeaderAdress));
		
	 } // end of while loop
	
	
} // end of _sweep_helper() 


// garbage collection: Mark and Sweep
void gc() {
	
	// calling mark helper function
	_mark_helper();
	// calling sweep helper function
	_sweep_helper();

}
