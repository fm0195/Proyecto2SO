#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "init.h"
#include "sharedMem.h"
int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Debe indicar la cantidad de lineas del archivo.\n");
    return 0;
  } else if (argc > 2){
    printf("Demasiados argumentos.\n");
    return 0;
  }
  int lines = atoi(argv[1]);
  init(lines);
  return 0;
}

void init(int lines){
  key_t key;
  int memId;
  int size;
  void* voidMem;
  struct SharedMem sharedMem;
  sharedMem.size = lines;
  size = sizeof(SharedMem)+sizeof(char)*LINE_LENGTH*MAX_LINES;
  key = ftok(MEM_DIR, MEM_KEY);
  memId = shmget(key, size, 0777 | IPC_CREAT);
  voidMem = shmat(memId, 0, 0);
  memset(voidMem, 0, size);
  memcpy(voidMem,&sharedMem,sizeof(SharedMem));
}
