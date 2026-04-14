#include <stdio.h>
#include <string.h>
#include "memory.h"

MemoryWord main_memory[MEMORY_SIZE];

void initializeMemory() {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        strcpy(main_memory[i].name, "EMPTY");
        strcpy(main_memory[i].value, "NULL");
    }
    printf("Memory successfully initialized with %d words.\n", MEMORY_SIZE);
}

void printMemory() {
    printf("\n=== CURRENT MEMORY STATE ===\n");
    for (int i = 0; i < MEMORY_SIZE; i++) {
        if (strcmp(main_memory[i].name, "EMPTY") != 0) {
            printf("Word %02d | Name: %-15s | Value: %s\n", i, main_memory[i].name, main_memory[i].value);
        }
    }
    printf("============================\n");
}