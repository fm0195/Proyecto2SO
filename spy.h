#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#include "sharedMem.h"
#endif


void getMem(SharedMem* sharedMem);
void* execSpy(SharedMem* mem);
char* readLine(SharedMem memory, int line);

int getState(SharedMem memory, int idProcess, int typeProcess);
void generateState(int state, int process);
