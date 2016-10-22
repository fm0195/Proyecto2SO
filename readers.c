#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include "sharedMem.h"
#include "readers.h"
int cantidad=0;
int sleepTime=0;
int readingTime=0;
int numReaders = 0;
pthread_mutex_t semMutex;

int main(int argc, char const *argv[]) {
  if(argc < 4){
    printf("Error, no se especifico los argumentos válidos.");
    return 0;
  }
  cantidad = atoi(argv[1]);
  if(cantidad < 1){
    printf("Error, la cantidad debe ser mayor que 0.");
    return 0;
  }
  sleepTime = atoi(argv[2]);
  if(sleepTime < 1){
    printf("Error, el tiempo de pausa de los hilos debe ser mayor que 0.");
    return 0;
  }
  readingTime = atoi(argv[3]);
  if(readingTime < 1){
    printf("Error, el tiempo de escritura de los hilos debe ser mayor que 0.");
    return 0;
  }

  struct SharedMem mem;
  getMem(&mem);
  pthread_mutex_init(&semMutex, NULL);
  startReaders(&mem);
  return 0;
}

void getMem(SharedMem* sharedMem){
  key_t key;
  int memId;
  int size;
  void* voidMem;
  char* res;
  size = sizeof(SharedMem)+sizeof(char)*LINE_LENGTH*MAX_LINES;
  key = ftok(MEM_DIR, MEM_KEY);
  memId = shmget(key, size, 0777 | IPC_CREAT);
  voidMem = shmat(memId, 0, 0);
  memcpy((void*)sharedMem,voidMem,sizeof(SharedMem));
  sharedMem->lines = malloc(sizeof(char*)*MAX_LINES);
  /*OBTENER SEMAFOROS*/
  sharedMem->semReaders = sem_open(SEM_READERS, 0);
  sharedMem->semWriters = sem_open(SEM_WRITERS, 0);
  sharedMem->semMutex = sem_open(SEM_MUTEX, 0);

  for (int i = 0; i < sharedMem->size; i++) {
    res = &(((char*)(voidMem+sizeof(SharedMem)))[i*LINE_LENGTH]);
    sharedMem->lines[i] = res;
  }
  sharedMem->isExecuting = voidMem+sharedMem->offset;
}

void *startReaders(SharedMem* mem) {
    int counter = 0;
    pthread_t tWriter;
    while (counter < cantidad) {
      Dto* dto = (struct Dto*)malloc(sizeof(struct Dto));
      dto->id=counter+1;
      dto->memory=mem;
      pthread_create(&tWriter, NULL, execReader,  dto);
      counter++;
    }
    pthread_join(tWriter,NULL);
    return 0;
}

void* execReader(Dto* dto){
  SharedMem* mem = dto->memory;
  int currentLine = 0;
  int semValue;
  while(*mem->isExecuting) {
    pthread_mutex_lock(&semMutex);//pido mutex para consultar numero de readers.
    sem_getvalue(mem->semWriters, &semValue);
    if (semValue == 1) {
      sem_wait(mem->semWriters);
    }
    if (numReaders++ == 0) {//Primer reader, pedir el semaforo
      pthread_mutex_unlock(&semMutex);//devuelvo mutex
      sem_wait(mem->semReaders);
      printf("Primer reader. Pedi semaforo\n");
    }else{
      pthread_mutex_unlock(&semMutex);//devuelvo mutex
    }
    if(*mem->lines[currentLine]){
      sleep(readingTime);
      printf("Read: %s . Reader Id: %i\n", readLine(*mem, currentLine), dto->id);
    }else{
      printf("Linea %i vacia . Reader Id: %i\n", currentLine, dto->id);
    }
    if (++currentLine >= mem->size) {
      currentLine = 0;
    }
    pthread_mutex_lock(&semMutex);//pido mutex para consultar numero de readers.
    if (--numReaders == 0){//Si soy el ultimo proceso
      pthread_mutex_unlock(&semMutex);//devuelvo mutex
      sem_post(mem->semReaders);//devuelvo el semaforo
      sem_post(mem->semWriters);//devuelvo el semaforo
      printf("Ultimo reader. Devolvi semaforos.\n");
    }else{
      pthread_mutex_unlock(&semMutex);//devuelvo mutex
    }
    sleep(sleepTime);
  }
  pthread_exit(NULL);
  return 0;
}

char* readLine(SharedMem memory, int line){
  if (line < memory.size) {
    return memory.lines[line];
  }
  return "ERROR. Out of bounds.";
}
