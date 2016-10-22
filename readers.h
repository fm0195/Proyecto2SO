#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#include "sharedMem.h"
#endif
void getMem(struct SharedMem* mem);
char* readLine(struct SharedMem memory, int line);
void* startReaders(struct SharedMem* mem);
void* execReader(struct Dto* dto);
int emptyLine(char* line);
