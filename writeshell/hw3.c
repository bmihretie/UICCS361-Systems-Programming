/* 

 cs 361 Homework3
 
 This homework is set up by professor  Sepideh Roghanchi and the code
 is written and submitted by Birhanu Mihretie

 The homweork  introduces how you can writie a shell program. A shell is the program that we can use when we ssh to servers. The program does
the following:
1. Prompt the user
2. Read and parse the command the user types in
3. Create a new process to run the command the user typed
4. Wait for the new process to end and print out its exit status
5. Start over with the prompt.

So I parse the input and split it by using strtok based on the newline or semi colon or pipe and then passes it to 2D argsarray with using strcpy.

Then I crete new process by uing fork and then make the parent to wait until the child finishes by using wait

Then I execute the command by using execvp so that it can print the out put and the status and pid for each process.

Finally, I implment the I/O redirection using dup2.The process that is running the first command should write its output to the write side of the pipe instead of stdout, and the process running the second command should read from the read side of the pipe instead of stdin.

 Finally I would like to give credit for Professor and  TA's for their great support that enable me to do the homework. I have also taken the idea and some of the codes to do my homework from professor Lectures, Lab materials and Computer Systems, a programmer's perspective (3rd edition)by Randal E. Bryant and David R O'Hallaron.
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include<sys/wait.h>
#include <sys/types.h>


// function to fork and execute the command for single command 
// and two commands separated by the semicolon
void  m_exec(char**argsarray){
  pid_t pid = fork(); // creating process
  if (pid == 0){ // child case
    execvp(argsarray[0], argsarray);
  }
  else{ // parent instance
    int status;
    wait(&status);
    printf("pid:%d status:%d\n",pid, WEXITSTATUS(status));
  }
}

// function to handle and change default behavior of the SIGINT signal
void _SIGINT(int sig){
  	printf("\ncaught sigint\n");
  	printf("CS361 >\n");
}

// function to handle and change default behavior of the SIGTSTP signal
void _SIGTSTP(int sig){
	printf( "\ncaught sigtstp\n");
  printf("CS361 >\n");
}


int main(){
 
  //printing  prompt and starting my shell
  printf("CS361 > ");
  signal(SIGINT,_SIGINT);
  signal(SIGTSTP,_SIGTSTP);

  // a loop to handle all the process commands
  while(1){

    // a pointer to handle the 2d array dynamic memory or malloc allocation
    char **argsarray;
    int row = 20;
    int col = 100;
    argsarray = malloc(row * sizeof(char *));
    for (int i=0; i < row; i++){
      argsarray[i] = malloc(col * sizeof(char));
    }

    // create some space for our strings
    char line[500];
    //read line from terminal
    fgets(line, 500, stdin);
    
    
    // a process that deals with two commands separated by semicolon 
    if(strchr(line,';') != NULL){
      // splitting the first command based on ';' and storing the left command before ';' to the pointer
      char *word = strtok(line, ";");
      // splitting the second command based on ';' and storing the right command after ';' to the pointer
      char *temp = strtok(NULL,";");

      // splitting,parssing the first command and copying it to argsarray
      char *word1 = strtok(word," \n");
      int i = 0;
      //copy a word to the arg array
      while(word1){
        strcpy(argsarray[i], word1);
        //get next word
        word1 = strtok(NULL, " \n");
        i = i + 1;

      } 
      // passing null character
      argsarray[i] = (char*)NULL;
      // calling execvp and fork method for the first command 
      m_exec(argsarray);
      //processing and passing the second command to argsarray
      char *temp1 = strtok(temp," \n");
      int k = 0;
      //copy a word to the arg array
      while(temp1){

        strcpy(argsarray[k],temp1);
        //get next word
        temp1 = strtok(NULL, " \n");
        k = k + 1;

      }
      // passing null character
      argsarray[i] = (char*)NULL;
      // calling execvp & fork method for the second command 
      m_exec(argsarray);
      
    } // end of else if(strchr(line,';') != NULL)

    // process that deals with pipe command 
    else if(strchr(line,'|') != NULL){

      int pipefds[2];
      pid_t pid,pid1;
      // parsing and spliting the left and right command in the pipe
      char *word = strtok(line, "|");
      char *temp = strtok(NULL,"|");

      // processing and passing the left side  command to argsarray
      char *word1 = strtok(word," ");
      int i = 0;
      //copy a word to the arg array
      while(word1){
        //printf("word %s\n",word1);
        strcpy(argsarray[i], word1);
        //get next word
        word1 = strtok(NULL, " ");

        i = i + 1;

      }
      // passing null character
      argsarray[i] = (char*)NULL;
      //create pipe
      pipe(pipefds);
      // creating process using fork
      pid = fork();
      //child case
      if (pid == 0) {

          //redirect the standard output
          dup2(pipefds[1],STDOUT_FILENO);
          //child close the write end
          close(pipefds[1]); //closed the 
          //after finishing reading, child close the read end
          close(pipefds[0]);
          // executing the first command
          execvp(argsarray[0], argsarray);
          
      }

      //parent case
      else {
        // creat process again
        pid1 = fork();
        //child case
        if(pid1 == 0){

          
          // redirecting the standard input
          dup2(pipefds[0], STDIN_FILENO);
          // closing the unused write end of the child
          close(pipefds[1]); 
          //parsing and passing to argsarray the right side command
          char *temp1 = strtok(temp," ");
          int k = 0;
          //copy a word to the arg array
          while(temp1){
            //printf("temp1 %s\n",temp1);
            strcpy(argsarray[k],temp1);
            //get next word
            temp1 = strtok(NULL, " ");
            k = k + 1;

          }

          // passing null character
          argsarray[i] = (char*)NULL;
          // executing the second command
          execvp(argsarray[0], argsarray);
        }
        // parent close the unused ends
        close(pipefds[0]);
        close(pipefds[1]);
        int status1,status2;
        // parent wait untill the children finishes
        wait(&status1);
        wait(&status2);
        // print the pid and exit status
        printf("pid:%d status:%d\n",pid, WEXITSTATUS(status1));
        printf("pid:%d status:%d\n",pid1,WEXITSTATUS(status2));

      }
           
    } // end of else if(strchr(line,'|') != NULL)
      
    else { // a process that handles a single command
      // parse and split the input based on newline chracter
      char *word3 = strtok(line, " \n");
      int i = 0;
      //copy a word to the arg array
      while(word3){
        // condtion to exit from the program
        if(strcmp(word3,"exit") == 0){
     		exit(0);
      	
   	    }
        
        strcpy(argsarray[i], word3);
        //get next word
        word3 = strtok(NULL, " \n");

        i = i + 1;

      }
      // passing null character
      argsarray[i] = (char*)NULL;
      // calling the function to execute the single command
      m_exec(argsarray);
    
    } // end of else condtions to handle single command

    //print prompt again
    printf("CS361 > ");

  } // end of while(1) condtions

  return 0;
 
} // end of main
