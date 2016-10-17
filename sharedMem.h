#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#define MEM_KEY 420
#define MEM_DIR "bin/ls"
#define LINE_LENGTH 0x80
#define MAX_LINES 100
/*CONSTANTES DE SEMAFOROS SE DEFINEN AQUI*/
#define SEM_WRITERS "/semwriter"
#define SEM_READERS "/semreader"
#include <semaphore.h>
typedef struct SharedMem {
  int size;
  int isExecuting;
  sem_t* semWriters;
  sem_t* semReaders;
  char** lines;
} SharedMem;
#endif
