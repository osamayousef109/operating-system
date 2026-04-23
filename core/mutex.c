#include "../scheduler/scheduler.h"
#include "mutex.h"

#include <stddef.h>

#include "queues.h"

mutex system_mutexes[3];

void init_mutexes() {
    system_mutexes[userInput].type = userInput;
    system_mutexes[userInput].available = 1;
    system_mutexes[userInput].blocked_queue.head = NULL;
    system_mutexes[userInput].blocked_queue.tail = NULL;

    system_mutexes[userOutput].type = userOutput;
    system_mutexes[userOutput].available = 1;
    system_mutexes[userOutput].blocked_queue.head = NULL;
    system_mutexes[userOutput].blocked_queue.tail = NULL;

    system_mutexes[file].type = file;
    system_mutexes[file].available = 1;
    system_mutexes[file].blocked_queue.head = NULL;
    system_mutexes[file].blocked_queue.tail = NULL;
}

void semWait(mutex* mutex) {
    if (mutex->available == 0) {
        push(running_process,&blockedQueue);
        push(running_process,&(mutex->blocked_queue));
        running_process->state = BLOCKED;
    }
    else {
        mutex->available = 0;
    }
}

void semSignal(mutex* mutex) {
    if (!empty(&(mutex->blocked_queue))) {
        int pid = pop(&(mutex->blocked_queue))->pid;
        PCB* next_process = removeProcessByPID(&blockedQueue,pid);
        push(next_process,&readyQueue);
        next_process->state = READY;
    }
    else {
        mutex->available = 1;
    }
}
