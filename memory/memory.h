#ifndef MEMORY_H
#define MEMORY_H

#include "pcb.h"

#define MEMORY_SIZE 40
#define VARIABLES_PER_PROCESS 3

typedef struct {
    char name[20];
    char value[50];
} MemoryWord;


extern MemoryWord main_memory[MEMORY_SIZE];

void initializeMemory();
void printMemory();

#endif