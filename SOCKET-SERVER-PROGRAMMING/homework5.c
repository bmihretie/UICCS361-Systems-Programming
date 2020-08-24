/* 
 cs 361 Homework5
 
This homework is set up and most of the codes is written by professor  Sepideh Roghanchi 
and the code is updated and submitted by Birhanu Mihretie. 

The homweork  introduces you how to follow a network protocol (in this case, http), learn 
network/socket programming and learn some simple multithreading.So, I included the code 
that can enable us to print out the correct mime type and http status response or errors.
I also included the code that enable us to do multi threading when the htpp request and 
is run between the client and server.


 Finally I would like to give credit for Professor and  TA's for their great support that 
 enable me to do the homework. I have also taken the idea and some of the codes to do my 
 homework from professor Lectures, Lab materials and Computer Systems, a programmer's 
 perspective (3rd edition)by Randal E. Bryant and David R O'Hallaron.
 */


#include <fnmatch.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BACKLOG (10)

//function declaration for serve_request
void serve_request(int); 

/* char* parseRequest(char* request)
 * Args: HTTP request of the form "GET /path/to/resource HTTP/1.X" 
 *
 * Return: the resource requested "/path/to/resource"
 *         0 if the request is not a valid HTTP request 
 * 
 * Does not modify the given request string. 
 * The returned resource should be free'd by the caller function. 
 */
char* parseRequest(char* request) {
  //assume file paths are no more than 256 bytes + 1 for null. 
  char *buffer = malloc(sizeof(char)*257);
  memset(buffer, 0, 257);
  
  if(fnmatch("GET * HTTP/1.*",  request, 0)) return 0; 

  sscanf(request, "GET %s HTTP/1.", buffer);
  return buffer; 
}


// a function that takes the fd of the client ,process it and output the http response from server
void serve_request(int client_fd){ 
	
  int read_fd;
  int bytes_read;
  int file_offset = 0;
  char client_buf[4096];
  struct stat sbuf;
  char send_buf[4096];
  char request_str[4096];
  char filename[4096];
  char * requested_file;
  memset(client_buf,0,4096);
  memset(filename,0,4096);
	
  while(1){

    file_offset += recv(client_fd,&client_buf[file_offset],4096,0);
    if(strstr(client_buf,"\r\n\r\n"))
      break;
  }
  
  // out put the correct cotnet type of http response based on the rquested file type
  requested_file = parseRequest(client_buf); 
  char* pdot = strrchr(requested_file,'.');
  char* mimetype = "text/html";
	
  if(pdot){
	  
	  if(strcmp(pdot,".jpg") == 0)
		  mimetype = "image/jpeg";
	  else if(strcmp(pdot,".gif") == 0)
		  mimetype = "image/gif";
	  else if(strcmp(pdot,".png") == 0)
		  mimetype = "image/png";
	  else if(strcmp(pdot,".ico") == 0)
                  mimetype = "image/x-icon";
	  else if(strcmp(pdot,".pdf") == 0)
		  mimetype = "application/pdf";
          else if(strcmp(pdot,".html") == 0)
          	  mimetype = "index/html";
	  else
		  mimetype = "text/plain";  
  }
  
	
 // output the correct error status or response to http response
 char* error_status = "200 OK";
 // a pointer that takes the file path for error checking
 char* filepath = requested_file + 1;
	
 // check the file path using  stat 
 if(stat(filepath,&sbuf) != -1){
    // is it a correct file path?
	if(S_ISREG(sbuf.st_mode))
            error_status = "200 OK";
	//if it is a is not a correct file path and and a directory    
	else if(!S_ISREG(sbuf.st_mode) && S_ISDIR(sbuf.st_mode))
	   error_status = "400 Bad Request";
  }
  // if the file is not found 
  else if(stat(filepath ,&sbuf) == -1){
      error_status = "404 Not Found";
  }
 
 // output the contet type and errorr status code in http response using sprintf
 sprintf(request_str,"HTTP/1.0 %s\r\n" "Content-type: %s; charset=UTF-8\r\n\r\n",error_status,mimetype);
   
	
  send(client_fd,request_str,strlen(request_str),0);
  // take requested_file, add a . to beginning, open that file
  filename[0] = '.';
  strncpy(&filename[1],requested_file,4095);
   
  read_fd = open(filename,0,0);
  while(1){
    bytes_read = read(read_fd,send_buf,4096);
    if(bytes_read <= 0)
      break;

    send(client_fd,send_buf,bytes_read,0);
  }
  close(read_fd);

  /* Tell the OS to clean up the resources associated with that client
   * connection, now that we're done with it. */
  close(client_fd);
  return;
}

// athread function that runs the thhread and calls serve_request
void* thread_t(void*ptr){
    serve_request(*(int*)ptr); // calling serve_request
    free(ptr); // freeing the memory allocated to the pointer
    
    return 0;
}

/* Your program should take two arguments:
 * 1) The port number on which to bind and listen for connections, and
 * 2) The directory out of which to serve files.
 */
int main(int argc, char** argv) {
    /* For checking return values. */
    int retval;

    /* Read the port number from the first command line argument. */
    int port = atoi(argv[1]);
	
    //read the second command line argument and change the directory
    chdir(argv[2]);
   
	
    /* Create a socket to which clients will connect. */
    int server_sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(server_sock < 0) {
        perror("Creating socket failed");
        exit(1);
    }

    /* A server socket is bound to a port, which it will listen on for incoming
     * connections.  By default, when a bound socket is closed, the OS waits a
     * couple of minutes before allowing the port to be re-used.  This is
     * inconvenient when you're developing an application, since it means that
     * you have to wait a minute or two after you run to try things again, so
     * we can disable the wait time by setting a socket option called
     * SO_REUSEADDR, which tells the OS that we want to be able to immediately
     * re-bind to that same port. See the socket(7) man page ("man 7 socket")
     * and setsockopt(2) pages for more details about socket options. */
    int reuse_true = 1;
    retval = setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_true,
                        sizeof(reuse_true));
    if (retval < 0) {
        perror("Setting socket option failed");
        exit(1);
    }

    /* Create an address structure.  This is very similar to what we saw on the
     * client side, only this time, we're not telling the OS where to connect,
     * we're telling it to bind to a particular address and port to receive
     * incoming connections.  Like the client side, we must use htons() to put
     * the port number in network byte order.  When specifying the IP address,
     * we use a special constant, INADDR_ANY, which tells the OS to bind to all
     * of the system's addresses.  If your machine has multiple network
     * interfaces, and you only wanted to accept connections from one of them,
     * you could supply the address of the interface you wanted to use here. */
    
   
    struct sockaddr_in6 addr;   // internet socket address data structure
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port); // byte order is significant
    addr.sin6_addr = in6addr_any; // listen to all interfaces

    
    /* As its name implies, this system call asks the OS to bind the socket to
     * address and port specified above. */
    retval = bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    if(retval < 0) {
        perror("Error binding to port");
        exit(1);
    }

    /* Now that we've bound to an address and port, we tell the OS that we're
     * ready to start listening for client connections.  This effectively
     * activates the server socket.  BACKLOG (#defined above) tells the OS how
     * much space to reserve for incoming connections that have not yet been
     * accepted. */
    retval = listen(server_sock, BACKLOG);
    if(retval < 0) {
        perror("Error listening for connections");
        exit(1);
    }

    while(1) {
        /* Declare a socket for the client connection. */
        int sock;

        /* Another address structure.  This time, the system will automatically
         * fill it in, when we accept a connection, to tell us where the
         * connection came from. */
        struct sockaddr_in remote_addr;
        unsigned int socklen = sizeof(remote_addr); 

        /* Accept the first waiting connection from the server socket and
         * populate the address information.  The result (sock) is a socket
         * descriptor for the conversation with the newly connected client.  If
         * there are no pending connections in the back log, this function will
         * block indefinitely while waiting for a client connection to be made.
         * */
        sock = accept(server_sock, (struct sockaddr*) &remote_addr, &socklen);
        if(sock < 0) {
            perror("Error accepting connection");
            exit(1);
        }

        /* At this point, you have a connected socket (named sock) that you can
         * use to send() and recv(). */

        /* craeat multithreading and run the thread */
        pthread_t thread;
	// create a pointer that points to accept fd
        int*psock = malloc(sizeof(int));
        *psock = sock;
        pthread_create(&thread,NULL,thread_t,psock);
        
    }

    close(server_sock);
	
}
