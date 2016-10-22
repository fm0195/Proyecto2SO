#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include "sharedMem.h"
#include "writers.h"
#include <unistd.h>

int cantidad=0;
int sleepTime=0;
int writeTime=0;

int main(int argc, char const *argv[]) {// ./writers.o cant sleepTime writeTime
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

  writeTime = atoi(argv[3]);
  if(writeTime < 1){
    printf("Error, el tiempo de escritura de los hilos debe ser mayor que 0.");
    return 0;
  }

  struct SharedMem mem;
  getMem(&mem);
  startWriters(&mem);
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

void *startWriters(SharedMem* mem) {
    int counter = 0;
    pthread_t tWriter;
    while (counter < cantidad) {
      Dto* dto = (struct Dto*)malloc(sizeof(struct Dto));
      dto->id=counter+1;
      dto->memory=mem;
      pthread_create(&tWriter, NULL, execWriter,  dto);
      counter++;
    }
    pthread_join(tWriter,NULL);
    return 0;
}

void* execWriter(Dto* dto){
  SharedMem* mem = dto->memory;
  while(*mem->isExecuting) {
    sem_wait(mem->semMutex);//pido mutex para consultar semaforo.
    int valueReader=0;
    sem_getvalue(mem->semReaders,&valueReader);
    if(valueReader <= 0){//si existen readers, dormir
        printf("El archivo se esta leyendo.\n");
        sem_post(mem->semMutex);//libero el semaforo para consultar semaforo.
        sem_wait(mem->semReaders);
        sem_post(mem->semReaders);
        continue;
    }
    sem_post(mem->semMutex);
    sem_wait(mem->semWriters);
    char str[LINE_LENGTH];
    int i;
    for (i=0; i < mem->size; i++) {
      if(emptyLine(readLine(*mem, i))){
        sprintf(str,"Linea %d, proceso %d",i,dto->id);

        time_t t = time(NULL);//consigo la fecha y hora del sistema
        struct tm *tm = localtime(&t);
        strcat(str,asctime(tm));

        writeLine(*mem, i, str, LINE_LENGTH);
        printf("%s%s\n", "Writing: ", mem->lines[i]);
        sleep(writeTime);
        break;
      }
    }
    if(i == mem->size){
      printf("%s\n", "Full file");
    }
    sem_post(mem->semWriters);
    sleep(sleepTime);
  }
  pthread_exit(NULL);
  return 0;
}

//Parametros: struct,       #linea,    puntero,       largo del string
void writeLine(SharedMem memory, int line, char* string, int size){
  if (line < memory.size) {
    memcpy(memory.lines[line], string, size);
    return;
  }
  printf("Line %i not written. Out of Bounds.\n", line);
}

char* readLine(SharedMem memory, int line){
  if (line < memory.size) {
    return memory.lines[line];
  }
  return "ERROR. Out of bounds.";
}

int emptyLine(char* line) {
    return *line==0;
}
