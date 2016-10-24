#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "sharedMem.h"
#include "finisher.h"

int main(int argc, char const *argv[]) {
  freeMem();
  return 0;
}

void freeMem(){
  key_t key;
  int memId;
  int size;
  size = sizeof(SharedMem)+sizeof(char)*LINE_LENGTH*MAX_LINES;
  key = ftok(MEM_DIR, MEM_KEY);
  memId = shmget(key, size, 0777 | IPC_CREAT);
  shmctl (memId , IPC_RMID , 0);
  sem_unlink(SEM_MUTEX);
  sem_unlink(SEM_READERS);
  sem_unlink(SEM_WRITERS);
}
