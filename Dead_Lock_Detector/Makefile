all:
	gcc -pthread ex2.c
	gcc -pthread ex1.c -o ex1
	gcc -pthread ex2.c -o ex2
	gcc -pthread -shared -fPIC -o ddmon.so ddmon.c -ldl
	gcc -pthread -o dinning dinning_deadlock.c

