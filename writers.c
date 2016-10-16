#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "sharedMem.h"
#include "writers.h"
#include <unistd.h>

int main(int argc, char const *argv[]) {
  struct SharedMem mem;
  getMem(&mem);
  sem_wait(mem.semWriters);
  writeLine(mem, 0, "Linea 0", 7);
  printf("%s%s\n", "Escrito: ", mem.lines[0]);
  printf("%s\n","Sleeping..." );
  sleep(5);
  printf("%s\n","Awake..." );
  sem_post(mem.semWriters);
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

  /*OBTENER SEMAFOROS*/
  sharedMem->semReaders = sem_open(SEM_READERS, 0);
  sharedMem->semWriters = sem_open(SEM_READERS, 0);
  
  for (int i = 0; i < sharedMem->size; i++) {
    char* res = &(((char*)(voidMem+sizeof(SharedMem)))[i*LINE_LENGTH]);
    sharedMem->lines[i] = res;
  }
}
//Parametros: struct,       #linea,    puntero,       largo del string
void writeLine(SharedMem memory, int line, char* string, int size){
  if (line < memory.size) {
    memcpy(memory.lines[line], string, size);
    return;
  }
  printf("Line %i not written. Out of Bounds.\n", line);
}
