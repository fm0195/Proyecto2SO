#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#include "sharedMem.h"
#endif

typedef struct DtoWritter {
  int id;
  SharedMem* memory;
} DtoWritter;

void getMem(struct SharedMem* mem);
void writeLine(struct SharedMem memory, int line, char* string, int size);
void startWriters(SharedMem* mem);
void execWriter(DtoWritter* dto);
char* readLine(struct SharedMem memory, int line);
int emptyLine(char* line);