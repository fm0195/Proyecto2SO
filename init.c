#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/stat.h>
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
  initLog();
  return 0;
}

void init(int lines){
  key_t key;
  int memId;
  int sizeOfMemory;//tamanno de la memoria por pedir
  int sizeOfStruct;//tamanno del struct
  int sizeOfLines;//tamanno en bytes de la zona para texto
  int sizeOfVariables;//tamanno en bytes de las variables compartidas;
  int offset;
  int isExecuting = 1;
  void* voidMem;
  struct SharedMem sharedMem;

  int sizeValues;

  sizeValues = sizeof(int)*MAX_PROCESS;
  sizeOfStruct = sizeof(SharedMem);

  sizeOfLines = sizeof(char)*LINE_LENGTH*MAX_LINES;
  sizeOfVariables = sizeof(int);//tamanno de cada una de las variables compartidas.
  sizeOfMemory = sizeOfStruct + sizeOfLines + sizeValues + 5 * sizeOfVariables;
  offset = sizeOfStruct + sizeOfLines + sizeValues;//lugar donde van a empezar las variables. Al final de las lineas.
  sharedMem.size = lines;
  sharedMem.offset = offset;


  /*INICIALIZAR LOS SEMAFOROS. ULTIMO PARAMETRO ES EL VALOR INICIAL*/
  sem_open(SEM_WRITERS, O_CREAT, 0644, 1);
  sem_open(SEM_READERS, O_CREAT, 0644, 1);
  sem_open(SEM_MUTEX, O_CREAT, 0644, 1);
  sem_open(SEM_INFO, O_CREAT, 0644, 1);
  sem_open(SEM_LOG, O_CREAT, 0644, 1);

  key = ftok(MEM_DIR, MEM_KEY);
  memId = shmget(key, sizeOfMemory, 0777 | IPC_CREAT);
  voidMem = shmat(memId, 0, 0);
  memset(voidMem, 0, sizeOfMemory);
  memcpy(voidMem,&sharedMem,sizeOfStruct);//copiar la estructura  los primeros sizeof(SharedMem) bytes.
  memcpy(voidMem+offset, &isExecuting, sizeof(int));//del inicio de la memoria compartida (voidMem), offset. Ahi va a quedar la variable.

  int initialValueAmountOfProcess=0;

  memcpy(voidMem+offset+sizeof(int), &initialValueAmountOfProcess, sizeof(int));
  memcpy(voidMem+offset+sizeof(int)*2, &initialValueAmountOfProcess, sizeof(int));
  memcpy(voidMem+offset+sizeof(int)*3, &initialValueAmountOfProcess, sizeof(int));
  memcpy(voidMem+offset+sizeof(int)*4, &initialValueAmountOfProcess, sizeof(int));
}
void initLog(){
  remove("Bitacora.txt");
  creat("Bitacora.txt", 0666);
}
