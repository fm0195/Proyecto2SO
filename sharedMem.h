#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#define MEM_KEY 420
#define MEM_DIR "bin/ls"
#define LINE_LENGTH 0x80
#define MAX_LINES 100
#define MAX_PROCESS 300
/*CONSTANTES DE SEMAFOROS SE DEFINEN AQUI*/
#define SEM_WRITERS "/semwriter"
#define SEM_READERS "/semreader"
#define SEM_MUTEX "/sem_mutex"
#define SEM_INFO "/sem_info"
#define SEM_LOG "/sem_log"
#include <semaphore.h>

typedef struct Dto {
  int id;
  struct SharedMem* memory;
} Dto;

typedef struct SharedMem {
  int size;
  int offset;
  int* isExecuting;
  sem_t* semWriters;
  sem_t* semReaders;
  sem_t* semMutex;
  sem_t* semInfo;
  sem_t* semLog;
  char** lines;
  int* amountReaders;
  int* amountWriters;
  int* amountSelfishReaders;
  int* selfishCounter;
  int** values;
} SharedMem;

#endif
