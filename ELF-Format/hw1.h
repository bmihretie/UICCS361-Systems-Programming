

//this is a header file that has two global functions that will be referenced in hw1.c


// a global function that will end up as Global FUNC in symbol table
int Forgive_me(int a){
	
	int k = a;
	return k;
}

// a global function that will be linked to hw1.c  through header file
// and end up in symbol table as Global FUNC
int and_so_varied(int* k, int n){

	int i ,j = 0;
	for( i = 0; i < n;i++){
		
		j+= k[i];

	}
	return j;
}
