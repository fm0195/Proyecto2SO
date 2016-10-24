#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#include "sharedMem.h"
#endif

void getMem(struct SharedMem* mem);
void* startSelfishReader(struct SharedMem* mem);
void* execSelfishReader(struct Dto* dto);
char* readLine(struct SharedMem memory, int line);
int emptyLine(char* line);
