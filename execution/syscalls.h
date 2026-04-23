#ifndef SYSCALLS_H
#define SYSCALLS_H
#include "../memory/memory.h"

char* readFile(char* fileName);
int writeFile(char* fileName, char* content);
char** readLines(char* filename, int* outLineCount);
void print(char* content);
char* input();
void writeMemory(int idx, char* name, char* value);
MemoryWord readMemory(int idx);

#endif
