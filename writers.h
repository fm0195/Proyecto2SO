#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#include "sharedMem.h"
#endif
void getMem(struct SharedMem* mem);
void write(struct SharedMem memory, int line, char* string, int size);
