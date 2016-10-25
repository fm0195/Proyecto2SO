#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include "sharedMem.h"
#include "spy.h"
#include <unistd.h>


int main(int argc, char const *argv[]) {// ./writers.o cant sleepTime readTime
  struct SharedMem mem;
  getMem(&mem);
  execSpy(&mem);
  return 0;
}

void getMem(SharedMem* sharedMem){
  key_t key;
  int memId;
  int size;
  void* voidMem;
  size = sizeof(SharedMem)+sizeof(char)*LINE_LENGTH*MAX_LINES;
  key = ftok(MEM_DIR, MEM_KEY);
  memId = shmget(key, size, 0777 | IPC_CREAT);
  voidMem = shmat(memId, 0, 0);
  memcpy((void*)sharedMem,voidMem,sizeof(SharedMem));
  sharedMem->lines = malloc(sizeof(char*)*MAX_LINES);
  sharedMem->values = malloc(sizeof(int*)*MAX_LINES);

  /*OBTENER SEMAFOROS*/
  sharedMem->semReaders = sem_open(SEM_READERS, 0);
  sharedMem->semWriters = sem_open(SEM_WRITERS, 0);
  sharedMem->semMutex = sem_open(SEM_MUTEX, 0);
  sharedMem->semInfo = sem_open(SEM_INFO, 0);
  char* res;
  for (int i = 0; i < sharedMem->size; i++) {
    res = &(((char*)(voidMem+sizeof(SharedMem)))[i*LINE_LENGTH]);
    sharedMem->lines[i] = res;
  }

  int* pointerValues=(int*)(res+LINE_LENGTH);
  for (int i = 0; i < MAX_PROCESS; i++) {
    sharedMem->values[i] = pointerValues;
    pointerValues += sizeof(int);
  }

  sharedMem->isExecuting = voidMem+sharedMem->offset;
  sharedMem->amountReaders = voidMem+sharedMem->offset +sizeof(int);
  sharedMem->amountWriters = voidMem+sharedMem->offset +(2*sizeof(int));
  sharedMem->amountSelfishReaders = voidMem+sharedMem->offset +(3*sizeof(int));
}

void* execSpy(SharedMem* mem){
   while(*mem->isExecuting) {
    sem_wait(mem->semInfo);
    int lines[mem->size];
    int i;
    int numLines = 0;
    printf("%s\n","Estado del archivo" );
    for (i=0; i < mem->size; i++) {
      char* lineValue = readLine(*mem, i);
      if(!(*lineValue)){
        printf("Linea %d: vacía. \n", i);
      }else printf("%s\n",lineValue );
    }
    printf("%s\n\n","************************************" );
    printf("%s\n","Estado de los procesos Lectores" );
    for (int i = 0; i < *mem->amountReaders; i++) {
      int state = getState(*mem, i, 0);
      generateState(state,i);
    }
    printf("%s\n\n","************************************" );
    printf("%s\n","Estado de los procesos Escritores" );
    for (int i = 0; i < *mem->amountWriters; i++) {
      int state = getState(*mem, i, 1);
      generateState(state,i);
    }
    printf("%s\n\n","************************************" );
    printf("%s\n","Estado de los procesos Lectores egoistas" );
    for (int i = 0; i < *mem->amountSelfishReaders; i++) {
      int state = getState(*mem, i, 2);
      generateState(state,i);
    }
    sem_post(mem->semInfo);
    break;
  }
  return 0;
}

char* readLine(SharedMem memory, int line){
  if (line < memory.size) {
    return memory.lines[line];
  }
  return "ERROR. Out of bounds.";
}

int getState(SharedMem memory, int idProcess, int typeProcess){ //0->Reader 1->Writer 2->selfishReader
  int index = ( typeProcess * 100 ) + idProcess;
  return *(memory.values[index]);
}
void generateState(int state, int process){
  switch (state) {
    case 0:
      printf("Proceso %d con estado Bloqueado. \n",process);
      break;
    case 1:
      printf("Proceso %d con estado Dormido. \n",process);
      break;
    case 2:
      printf("Proceso %d esta usando el archivo. \n",process);
      break;
  }
}
