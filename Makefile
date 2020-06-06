all:
	gcc -pthread -o abba abba.c
	gcc -pthread -shared -fPIC -o ddmon.so ddmon.c -ldl
