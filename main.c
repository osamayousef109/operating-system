#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "mutex.h"
#include "scheduler/scheduler.h"

int main() {
    printf("Booting Simulated OS...\n");
    

    initializeMemory();
    
    printMemory();
    init_mutexes();
    memset(main_memory, 0, sizeof(main_memory));
    runOS(RR);
    
    return 0;
}