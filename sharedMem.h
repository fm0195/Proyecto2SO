#ifndef SHAREDMEM_H
#define SHAREDMEM_H
#define MEM_KEY 420
#define MEM_DIR "bin/ls"
#define LINE_LENGTH 0x80
#define MAX_LINES 100
typedef struct SharedMem {
  int size;
  char** lines;
} SharedMem;
#endif
