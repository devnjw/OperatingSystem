all: smalloc.h smalloc2.c test1.c test2.c test3.c test4.c
	gcc -o test1 test1.c smalloc2.c
	gcc -o test2 test2.c smalloc2.c
	gcc -o test3 test3.c smalloc2.c 
	gcc -o test4 test4.c smalloc2.c 

clean:
	rm -rf test1 test2 test3 test4 smalloc.o
