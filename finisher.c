#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "sharedMem.h"
int memId;
void* voidMem;
int main(int argc, char const *argv[]) {
  struct SharedMem mem;
  getMem(&mem);
  int val=0;
  *mem.isExecuting = 0;
  sem_unlink(SEM_INFO);
  sem_unlink(SEM_MUTEX);
  sem_unlink(SEM_LOG);
  sem_unlink(SEM_READERS);
  sem_unlink(SEM_WRITERS);
  shmctl(memId, IPC_RMID, NULL);
  return 0;
}

void getMem(SharedMem* sharedMem){
  key_t key;
  int size;
  char* res;
  size = sizeof(SharedMem)+sizeof(char)*LINE_LENGTH*MAX_LINES + sizeof(int)*MAX_PROCESS;
  key = ftok(MEM_DIR, MEM_KEY);
  memId = shmget(key, size, 0777 | IPC_CREAT);
  voidMem = shmat(memId, 0, 0);
  memcpy((void*)sharedMem,voidMem,sizeof(SharedMem));
  sharedMem->lines = malloc(sizeof(char*)*MAX_LINES);
  /*OBTENER SEMAFOROS*/
  sharedMem->semReaders = sem_open(SEM_READERS, 0);
  sharedMem->semWriters = sem_open(SEM_WRITERS, 0);

  sharedMem->semMutex = sem_open(SEM_MUTEX, 0);
  sharedMem->semInfo = sem_open(SEM_INFO, 0);

  for (int i = 0; i < sharedMem->size; i++) {
    res = &(((char*)(voidMem+sizeof(SharedMem)))[i*LINE_LENGTH]);
    sharedMem->lines[i] = res;
  }
  sharedMem->isExecuting = voidMem+sharedMem->offset;
}
