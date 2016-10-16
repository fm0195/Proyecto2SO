init: init.c
	gcc -o init init.c -I.
readers: init.c
	gcc -o readers readers.c -I.
