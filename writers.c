#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "sharedMem.h"
#include "writers.h"
#include <unistd.h>

int cantidad=0;
int tiempoDormir=0;
int tiempoEscribir=0;
int lineaArchivo=0;//creo que esta variable es local a la funcion de cada thread.
pthread_t idEscritores[500];
pthread_mutex_t semaphoreMutex;

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
  tiempoDormir = atoi(argv[2]);
  if(tiempoDormir < 1){
    printf("Error, el tiempo de pausa de los hilos debe ser mayor que 0.");
    return 0;
  }

  tiempoEscribir = atoi(argv[3]);
  if(tiempoEscribir < 1){
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

  for (int i = 0; i < sharedMem->size; i++) {
    char* res = &(((char*)(voidMem+sizeof(SharedMem)))[i*LINE_LENGTH]);
    sharedMem->lines[i] = res;
  }
}

void *startWriters(SharedMem* mem) {
    int counter = 0;
    while (counter<cantidad) {
      pthread_t tWriter;
      DtoWritter* dto = (struct DtoWritter*)malloc(sizeof(struct DtoWritter));
      dto->id=counter+1;
      dto->memory=mem;
      pthread_create(&tWriter, NULL, execWriter,  dto);
      idEscritores[counter++]=tWriter;
    }
    while(mem->isExecuting);
    return 0;
}

void* execWriter(DtoWritter* dto){
  SharedMem* mem = dto->memory;
  while(mem->isExecuting) {
    pthread_mutex_lock(&semaphoreMutex);
    int valueReader=0;
    sem_getvalue(mem->semReaders,&valueReader);
    if(valueReader <= 0 || lineaArchivo == mem->size){
        printf("Archivo lleno o esta siendo leido.\n");
        pthread_mutex_unlock(&semaphoreMutex);
        sleep(tiempoDormir);
        continue;
    }
    sem_wait(mem->semWriters);
    pthread_mutex_unlock(&semaphoreMutex);
    char str[25];

    sprintf(str,"Linea %d, proceso %d",lineaArchivo,dto->id);
    writeLine(*mem, lineaArchivo, str,25);
    printf("%s%s\n", "Writing: ", mem->lines[lineaArchivo]);
    sleep(tiempoEscribir);
    lineaArchivo++;
    printf("%s %d\n","Sleeping... id:",dto->id );
    sem_post(mem->semWriters);
    sleep(tiempoDormir);
    printf("%s %d\n","Awake...id:",dto->id  );
  }
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
