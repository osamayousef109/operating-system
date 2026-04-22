#ifndef MUTEX_H
#define MUTEX_H

#include "pcb.h"
#include "queues.h"

// Resource types
typedef enum {
    FILE_RESOURCE,
    USER_INPUT_RESOURCE,
    USER_OUTPUT_RESOURCE
} ResourceType;

// Mutex structure
typedef struct {
    ResourceType resource_type;
    char resource_name[20];
    int owner_pid;              // PID of process holding the resource (-1 if free)
    Queue blocked_queue;        // Processes blocked on this resource
    int available;              // 1 if available, 0 if held by a process
} Mutex;

// Global mutexes (one for each resource)
extern Mutex file_mutex;
extern Mutex user_input_mutex;
extern Mutex user_output_mutex;

// Function declarations
void initializeMutexes();
Mutex* getMutex(ResourceType resource_type);
int semWait(ResourceType resource_type, PCB* process);
void semSignal(ResourceType resource_type);
void printMutexStatus();

#endif
