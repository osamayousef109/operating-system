#ifndef SYSCALLS_H
#define SYSCALLS_H
#include "../memory/memory.h"

char* readFile(char* fileName);
void writeFile(char* fileName, char* content);
void print(char* content);
char* input();
void writeMemory(int idx, char* name, char* value);
MemoryWord readMemory(int idx);

#endif //SYSCALL S_H
