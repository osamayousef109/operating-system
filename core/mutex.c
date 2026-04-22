#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mutex.h"

// Global mutex instances
Mutex file_mutex;
Mutex user_input_mutex;
Mutex user_output_mutex;

extern Queue readyQueue;
extern Queue blockedQueue;

/**
 * Initialize all mutexes
 * All resources start as available (not owned)
 */
void initializeMutexes() {
    // Initialize file mutex
    file_mutex.resource_type = FILE_RESOURCE;
    strcpy(file_mutex.resource_name, "file");
    file_mutex.owner_pid = -1;
    file_mutex.available = 1;
    file_mutex.blocked_queue.head = NULL;
    file_mutex.blocked_queue.tail = NULL;

    // Initialize user input mutex
    user_input_mutex.resource_type = USER_INPUT_RESOURCE;
    strcpy(user_input_mutex.resource_name, "userInput");
    user_input_mutex.owner_pid = -1;
    user_input_mutex.available = 1;
    user_input_mutex.blocked_queue.head = NULL;
    user_input_mutex.blocked_queue.tail = NULL;

    // Initialize user output mutex
    user_output_mutex.resource_type = USER_OUTPUT_RESOURCE;
    strcpy(user_output_mutex.resource_name, "userOutput");
    user_output_mutex.owner_pid = -1;
    user_output_mutex.available = 1;
    user_output_mutex.blocked_queue.head = NULL;
    user_output_mutex.blocked_queue.tail = NULL;

    printf("Mutexes initialized successfully.\n");
}

/**
 * Get mutex by resource type
 */
Mutex* getMutex(ResourceType resource_type) {
    switch (resource_type) {
        case FILE_RESOURCE:
            return &file_mutex;
        case USER_INPUT_RESOURCE:
            return &user_input_mutex;
        case USER_OUTPUT_RESOURCE:
            return &user_output_mutex;
        default:
            return NULL;
    }
}

/**
 * Semaphore Wait (Acquire) operation
 * 
 * If resource is available:
 *   - Acquire it and return success (1)
 * 
 * If resource is NOT available:
 *   - Block the process
 *   - Add to mutex's blocked queue AND general blocked queue
 *   - Return failure (0)
 */
int semWait(ResourceType resource_type, PCB* process) {
    Mutex* mutex = getMutex(resource_type);
    
    if (mutex == NULL) {
        printf("ERROR: Invalid resource type\n");
        return 0;
    }

    if (mutex->available == 1) {
        // Resource is available - acquire it
        mutex->available = 0;
        mutex->owner_pid = process->pid;
        printf("Time: Process %d acquired %s.\n", process->pid, mutex->resource_name);
        return 1;  // Success
    } else {
        // Resource is NOT available - block the process
        printf("Time: Process %d blocked on %s (held by P%d).\n", 
               process->pid, mutex->resource_name, mutex->owner_pid);
        
        process->state = BLOCKED;
        
        // Add to this specific mutex's blocked queue
        push(process, &mutex->blocked_queue);
        
        // Also add to general blocked queue
        push(process, &blockedQueue);
        
        return 0;  // Failure - process blocked
    }
}

/**
 * Semaphore Signal (Release) operation
 * 
 * 1. Release the resource (mark as available, clear owner)
 * 2. Check if any processes are waiting on this resource
 * 3. If yes, move the first one back to READY queue (wake it up)
 */
void semSignal(ResourceType resource_type) {
    Mutex* mutex = getMutex(resource_type);
    
    if (mutex == NULL) {
        printf("ERROR: Invalid resource type\n");
        return;
    }

    if (mutex->owner_pid == -1) {
        printf("WARNING: Trying to release unowned resource %s\n", mutex->resource_name);
        return;
    }

    int releaser_pid = mutex->owner_pid;
    
    // Release the resource
    mutex->available = 1;
    mutex->owner_pid = -1;
    printf("Time: Process %d released %s.\n", releaser_pid, mutex->resource_name);

    // Check if there are blocked processes waiting for this resource
    if (!empty(&mutex->blocked_queue)) {
        PCB* waiting_process = pop(&mutex->blocked_queue);
        
        if (waiting_process != NULL) {
            // Remove from general blocked queue as well
            removeProcessByPID(&blockedQueue, waiting_process->pid);
            
            // Wake up the waiting process
            waiting_process->state = READY;
            push(waiting_process, &readyQueue);
            printf("Time: Process %d woken up from %s and added to Ready Queue.\n", 
                   waiting_process->pid, mutex->resource_name);
        }
    }
}

/**
 * Print status of all mutexes for debugging
 */
void printMutexStatus() {
    printf("\n=== MUTEX STATUS ===\n");
    
    // File mutex
    printf("File Mutex:\n");
    printf("  Status: %s\n", file_mutex.available ? "AVAILABLE" : "HELD");
    printf("  Owner: P%d\n", file_mutex.owner_pid >= 0 ? file_mutex.owner_pid : -1);
    printf("  Blocked Processes: ");
    print_queue(&file_mutex.blocked_queue, "");
    
    // User Input mutex
    printf("User Input Mutex:\n");
    printf("  Status: %s\n", user_input_mutex.available ? "AVAILABLE" : "HELD");
    printf("  Owner: P%d\n", user_input_mutex.owner_pid >= 0 ? user_input_mutex.owner_pid : -1);
    printf("  Blocked Processes: ");
    print_queue(&user_input_mutex.blocked_queue, "");
    
    // User Output mutex
    printf("User Output Mutex:\n");
    printf("  Status: %s\n", user_output_mutex.available ? "AVAILABLE" : "HELD");
    printf("  Owner: P%d\n", user_output_mutex.owner_pid >= 0 ? user_output_mutex.owner_pid : -1);
    printf("  Blocked Processes: ");
    print_queue(&user_output_mutex.blocked_queue, "");
    
    printf("===================\n\n");
}
