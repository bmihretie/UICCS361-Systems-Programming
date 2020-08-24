/* 
 
 cs 361 Homework1
 
 This homework is set up by professor  Sepideh Roghanchi and the code
 is written and submitted by Birhanu Mihretie

 The homweork  introduces you how you can understand the linking process
 by using readelf. I have defined and declared both global and  static
 functions and  varaibles in hw1.c to match with the symbol table provided.
 I have also added  two global functions in the hw1.h

 Finally I would like to give credit for Professor Lecture slides and Computer
 Systems, a programmer's perspective (3rd edition) by Randal E. Bryant and
 David R O'Hallaron since I take the idea and some of the codes to write
 my function declarations and deefintions. 

 
 */



#include <stdio.h>
#include "hw1.h"

// static function with int return type which will be seen ass local FUNC
// in symbol table
static int I_have_written(){

	int m =2;
	return m;
}

// static array declaration which will be seen as local object in 
// symbol table
static int the_code[3] = {1,2,3};

// static function which will be seen as locl FUNC in symbol table
static int that_you_needed(int*a,int n){

	int i,s = 0;
	for(i = 0; i < n;i++){

		s-=a[i];
	
	}
	return s;

}

// a static function with number of stitic variables that produces
// a local FUNC and local objects with number of random numbers
static int and_which(){

       //to_compile,has_a_bunch_of and ridculus are static variables which
       // will end up in symbol table as local object with NDX value of 3
       // since they are  intialsed(data)
	static int to_compile = 3;
	static int has_a_bunch_of = 2;
	static int ridiculous = 1;
	static int symbols[2];
	
	symbols[1] = has_a_bunch_of + ridiculous;
	
	return symbols[1] + ridiculous + to_compile;

}


// another void static function whihc will be seen as local FUNC in 
// symbol table
void sides_and(){
	
	int j = 1;
	printf("%d\n",j);

} 

// global variable that will end up in symbol table as global object with 
// NDX value of COM since it is unintilaised
int they_are_arbitrary;

// global variable whihc will end up in symbol table as global object with 
// NDX value of 3 since it is intialsed(data)
int so_random[2] = {1,5};


int main(int argc,char* argv[]){
 
  // causes UIC netID to be printed on the first line of output when the
  // is run 
  printf("bmihre2");
  // call to I_have_written function
  I_have_written();
  // call to and_which function
  and_which();
  // call to that_you_needed function
  that_you_needed(the_code,2);

  return 0;

}
