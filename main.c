#include <stdio.h>
#include <string.h>
#include "memory.h"

int main() {
    printf("Booting Simulated OS...\n");
    

    initializeMemory();
    

    strcpy(main_memory[0].name, "x");
    strcpy(main_memory[0].value, "10");
    
    printMemory();
    
    return 0;
}