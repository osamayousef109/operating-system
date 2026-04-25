#include <stdio.h>
#include "memory/memory.h"
#include "core/mutex.h"
#include "scheduler/scheduler.h"

int main() {
    printf("Booting Simulated OS...\n");
    

    initializeMemory();
    
    printMemory();
    init_mutexes();
    runOS(RR);
    
    return 0;
}