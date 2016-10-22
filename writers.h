#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#include "sharedMem.h"
#endif

void getMem(struct SharedMem* mem);
void writeLine(struct SharedMem memory, int line, char* string, int size);
void* startWriters(struct SharedMem* mem);
void* execWriter(struct Dto* dto);
char* readLine(struct SharedMem memory, int line);
int emptyLine(char* line);
