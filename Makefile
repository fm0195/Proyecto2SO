all: init.c readers.o readers.c
	gcc -o init.o init.c -I.
	gcc -o readers.o readers.c -I.
	gcc -o writers.o writers.c -I.
	./init.o 50
clean:
	rm init.o readers.o writers.o
	ipcrm -M 0xffffffff
readers:
	gcc -o readers.o readers.c -I.
init:
	gcc -o init.o init.c -I.
writers:
	gcc -o writers.o writers.c -I.
