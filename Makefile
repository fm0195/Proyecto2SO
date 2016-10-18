all: init.c readers.o readers.c
	gcc -pthread -o init.o init.c -I.
	gcc -pthread -o readers.o readers.c -I.
	gcc -pthread -o writers.o writers.c -I.
	./init.o 50
clean:
	rm init.o readers.o writers.o
	ipcrm -M 0xffffffff
readers:
	gcc -pthread -o readers.o readers.c -I.
init:
	gcc -pthread -o init.o init.c -I.
writers:
	gcc -pthread -o writers.o writers.c -I.
