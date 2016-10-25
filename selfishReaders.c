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

void* voidMem;
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
  memcpy(mem.amountSelfishReaders,&cantidad,sizeof(int));
  startSelfishReader(&mem);
  return 0;
}

void getMem(SharedMem* sharedMem){
  key_t key;
  int memId;
  int size;
  size = sizeof(SharedMem)+sizeof(char)*LINE_LENGTH*MAX_LINES+ sizeof(int)*MAX_PROCESS;
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
  char* res;
  for (int i = 0; i < sharedMem->size; i++) {
    res = &(((char*)(voidMem+sizeof(SharedMem)))[i*LINE_LENGTH]);
    sharedMem->lines[i] = res;
  }

  int* pointerValues=(int*)(res+LINE_LENGTH);
  for (int i = 0; i < MAX_PROCESS; i++) {
    sharedMem->values[i] = pointerValues;
    pointerValues  += sizeof(int);
  }

  sharedMem->amountSelfishReaders = voidMem+sharedMem->offset +(3*sizeof(int));
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
    shmdt(voidMem);
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

    sem_wait(mem->semInfo);
    changeState(*mem,dto->id,2);
    sem_post(mem->semInfo);

    int lines[mem->size];
    int i;
    int numLines = 0;
    for (i=0; i < mem->size; i++) {
      if(! emptyLine(readLine(*mem, i))){
        lines[numLines++] = i;
      }
    }
    char outputLine[LINE_LENGTH+70];
    if(numLines == 0){
      sprintf(outputLine, "Empty file---- Selfish Id: %i",dto->id);
      printf("%s\n",outputLine );
    }else{
      int res = numLines == 1 ? 0 : rand() % (numLines - 1);
      res = lines[res];
      sleep(readTime);
      sprintf(outputLine,"Read: %s . ---- Selfish Id: %i \n", readLine(*mem, res), dto->id);
      printf("%s\n",outputLine );
      deleteLine(*mem, res);
      printf("Line %i deleted.\n", res);
      strcat(outputLine,"Borrado.\n");
    }
    sem_wait(mem->semLog);
    appendLineFile(outputLine);
    sem_post(mem->semLog);

    sem_post(mem->semWriters);

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

void deleteLine(SharedMem memory, int line){
  if (line < memory.size) {
    memset(memory.lines[line], 0, LINE_LENGTH);
  }
}

int emptyLine(char* line) {
    return *line==0;
}

void changeState(SharedMem memory, int idProcess, int state){ // 0->Bloqueado  1 -> durmiendo 2-> Usando el archivo
   if (idProcess < cantidad){
     int index = idProcess+199;
     memcpy(memory.values[index], &state, sizeof(int));
   }
}

void appendLineFile(char* line){
  FILE *out = fopen("Bitacora.txt", "a");
  fprintf(out, "%s", line);
  fclose(out);
  return 0;
}
