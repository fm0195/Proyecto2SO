#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include "sharedMem.h"
#include "selfishReaders.h"
#include <unistd.h>

int cantidad=0;
int sleepTime=0;
int readTime=0;

int main(int argc, char const *argv[]) {// ./writers.o cant sleepTime readTime
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

  readTime = atoi(argv[3]);
  if(readTime < 1){
    printf("Error, el tiempo de escritura de los hilos debe ser mayor que 0.");
    return 0;
  }

  struct SharedMem mem;
  getMem(&mem);
  startSelfishReader(&mem);
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
  sharedMem->semWriters = sem_open(SEM_WRITERS, 0);
  sharedMem->semMutex = sem_open(SEM_MUTEX, 0);

  for (int i = 0; i < sharedMem->size; i++) {
    char* res = &(((char*)(voidMem+sizeof(SharedMem)))[i*LINE_LENGTH]);
    sharedMem->lines[i] = res;
  }
  sharedMem->isExecuting = voidMem+sharedMem->offset;
}

void *startSelfishReader(SharedMem* mem) {
    int counter = 0;
    pthread_t tWriter;
    while (counter < cantidad) {
      Dto* dto = (struct Dto*)malloc(sizeof(struct Dto));
      dto->id=counter+1;
      dto->memory=mem;
      pthread_create(&tWriter, NULL, execSelfishReader,  dto);
      counter++;
    }
    pthread_join(tWriter,NULL);
    return 0;
}

void* execSelfishReader(Dto* dto){
  srand(time(NULL));
  SharedMem* mem = dto->memory;
  while(*mem->isExecuting) {
    sem_wait(mem->semMutex);//pido mutex para consultar semaforo.
    int valueReader=0;
    sem_getvalue(mem->semReaders,&valueReader);
    if(valueReader <= 0){//si existen readers, dormir
        printf("El archivo se esta leyendo. Esperando...\n");
        sem_post(mem->semMutex);//libero el semaforo para consultar semaforo.
        sem_wait(mem->semReaders);
        sem_post(mem->semReaders);
        printf("Archivo liberado.\n");
        continue;
    }
    sem_post(mem->semMutex);
    sem_wait(mem->semWriters);
    int lines[mem->size];
    int i;
    int numLines = 0;
    for (i=0; i < mem->size; i++) {
      if(! emptyLine(readLine(*mem, i))){
        lines[numLines++] = i;
      }
    }
    if(numLines == 0){
      printf("%s\n", "Empty file");
    }else{
      int res = numLines == 1 ? 0 : rand() % (numLines - 1);
      res = lines[res];
      sleep(readTime);
      printf("Read: %s . ---- Selfish Id: %i \n", readLine(*mem, res), dto->id);
      deleteLine(*mem, res);
      printf("Line %i deleted.\n", res);
    }
    sem_post(mem->semWriters);
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

void deleteLine(SharedMem memory, int line){
  if (line < memory.size) {
    memset(memory.lines[line], 0, LINE_LENGTH);
  }
}

int emptyLine(char* line) {
    return *line==0;
}
