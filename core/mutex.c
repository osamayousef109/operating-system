#include "../scheduler/scheduler.h"
#include "mutex.h"

#include "queues.h"

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
