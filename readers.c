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
   memcpy(mem.amountReaders,&cantidad,sizeof(int));
  //sem_post(mem.semWriters);
  //sem_post(mem.semReaders);
  // int valueReader=0;
  // sem_getvalue(mem.semReaders,&valueReader);
  // printf("R: %d\n", valueReader);
  // sem_getvalue(mem.semMutex,&valueReader);
  // printf("M: %d\n", valueReader);
  // sem_getvalue(mem.semWriters,&valueReader);
  // printf("W: %d\n", valueReader);
  // sem_getvalue(mem.semInfo,&valueReader);
  // printf("I: %d\n", valueReader);
  startReaders(&mem);
  return 0;
}

void getMem(SharedMem* sharedMem){
  key_t key;
  int memId;
  int size;
  void* voidMem;
  char* res;
  size = sizeof(SharedMem)+sizeof(char)*LINE_LENGTH*MAX_LINES + sizeof(int)*MAX_PROCESS;
  key = ftok(MEM_DIR, MEM_KEY);
  memId = shmget(key, size, 0777 | IPC_CREAT);
  voidMem = shmat(memId, 0, 0);
  memcpy((void*)sharedMem,voidMem,sizeof(SharedMem));
  sharedMem->lines = malloc(sizeof(char*)*MAX_LINES);
  sharedMem->values = malloc(sizeof(int*)*MAX_PROCESS);
  /*OBTENER SEMAFOROS*/
  sharedMem->semReaders = sem_open(SEM_READERS, 0);
  sharedMem->semWriters = sem_open(SEM_WRITERS, 0);
  sharedMem->semMutex = sem_open(SEM_MUTEX, 0);
  sharedMem->semInfo = sem_open(SEM_INFO, 0);
  sharedMem->semLog = sem_open(SEM_LOG, 0);

  for (int i = 0; i < sharedMem->size; i++) {
    res = &(((char*)(voidMem+sizeof(SharedMem)))[i*LINE_LENGTH]);
    sharedMem->lines[i] = res;
  }

  int* pointerValues=(int*)(res+LINE_LENGTH);
  for (int i = 0; i < MAX_PROCESS; i++) {
    sharedMem->values[i] = pointerValues;
    pointerValues  += sizeof(int);
  }

  sharedMem->isExecuting = voidMem+sharedMem->offset;
  sharedMem->amountReaders = voidMem+sharedMem->offset +sizeof(int);

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
  while(*mem->isExecuting) {
    pthread_mutex_lock(&semMutex);//pido mutex para consultar numero de readers.
    if (numReaders++ == 0) {//Primer reader, pedir el semaforo
      sem_wait(mem->semWriters);
      sem_wait(mem->semReaders);
      printf("\n---------------------\n");
      pthread_mutex_unlock(&semMutex);//devuelvo mutex
    }else{
      pthread_mutex_unlock(&semMutex);//devuelvo mutex
    }
    sem_wait(mem->semInfo);
    changeState(*mem,dto->id,2);
    sem_post(mem->semInfo);
    char outputLine[LINE_LENGTH+70];
    if(*mem->lines[currentLine]){
      sleep(readingTime);
      sprintf(outputLine,"Read: %s . ---- Reader Id: %i ", readLine(*mem, currentLine), dto->id);
    }else{
      sprintf(outputLine,"Linea %i vacia. ---- Reader Id: %i ", currentLine, dto->id);
    }
    printf("%s\n", outputLine);
    strcat(outputLine," Lectura.\n");
    sem_wait(mem->semLog);
    appendLineFile(outputLine);
    sem_post(mem->semLog);

    if (++currentLine >= mem->size) {
      currentLine = 0;
    }
    pthread_mutex_lock(&semMutex);//pido mutex para consultar numero de readers.
    if (--numReaders == 0){//Si soy el ultimo proceso
      sem_post(mem->semReaders);//devuelvo el semaforo
      sem_post(mem->semWriters);//devuelvo el semaforo
      printf("\n---------------------\n");
      pthread_mutex_unlock(&semMutex);//devuelvo mutex
    }else{
      pthread_mutex_unlock(&semMutex);//devuelvo mutex
    }
    sem_wait(mem->semInfo);
    changeState(*mem,dto->id,1);
    sem_post(mem->semInfo);

    sleep(sleepTime);

    sem_wait(mem->semInfo);
    changeState(*mem,dto->id,0);
    sem_post(mem->semInfo);
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

void changeState(SharedMem memory, int idProcess, int state){ // 0->Bloqueado  1 -> durmiendo 2-> Usando el archivo
   int amountReaders = *(memory.amountReaders);
   if (idProcess <= amountReaders){
     memcpy(memory.values[idProcess-1], &state, sizeof(int));
   }
}

void appendLineFile(char* line){
  FILE *out = fopen("Bitacora.txt", "a");
  fprintf(out, "%s", line);
  fclose(out);
  return 0;
}
