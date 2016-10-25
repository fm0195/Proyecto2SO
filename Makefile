all: init.c readers.o readers.c
	gcc -pthread -o init.o init.c -I.
	gcc -pthread -o readers.o readers.c -I.
	gcc -pthread -o writers.o writers.c -I.
	gcc -pthread -o selfishReaders.o selfishReaders.c -I.
	gcc -pthread -o finisher.o finisher.c -I.
	gcc -pthread -o spy.o spy.c -I.
	./init.o 30
clean:
	rm init.o readers.o writers.o selfishReaders.o finisher.o
	./finisher.o
readers:
	gcc -pthread -o readers.o readers.c -I.
init:
	gcc -pthread -o init.o init.c -I.
writers:
	gcc -pthread -o writers.o writers.c -I.
finisher:
	gcc -pthread -o finisher.o finisher.c -I.
