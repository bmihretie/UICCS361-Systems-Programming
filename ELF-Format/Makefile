hw1: hw1.o
	@gcc -Wall -o hw1 hw1.o

hw1.o : hw1.c hw1.h
	@gcc -c -Wall hw1.c

.PHONY:test
test: hw1.o
	@readelf -s hw1.o

clean:
	rm hw1 hw1.o
