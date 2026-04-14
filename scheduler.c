#include <stdio.h>
#include <stdlib.h>
#include "pcb.h"
#include "queues.h"
#include "scheduler.h"

int system_time = 0;
PCB* running_process = NULL;
const int time_slice = 2;

Queue mlfq[4] = {
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL}
};

void print_queues(Algorithm algo) {
    if (algo == MLFQ) {
        print_queue(&mlfq[0], "MLFQ Level 0");
        print_queue(&mlfq[1], "MLFQ Level 1");
        print_queue(&mlfq[2], "MLFQ Level 2");
        print_queue(&mlfq[3], "MLFQ Level 3");
    } else {
        print_queue(&readyQueue, "Ready Queue");
    }
    printf("--------------------------------------------------\n");
}

PCB* create_process(int id, int arrival, int burst) {
    PCB* p = (PCB*)malloc(sizeof(PCB));
    p->pid = id;
    p->state = READY;
    p->pc = 0;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->time_in_cpu = 0;
    p->queue_level = 0;
    return p;
}

PCB* pop_HRRN(Queue* queue, int systemTime) {
    if (empty(queue)) return NULL;
    Node* current = queue->head;
    Node* prev = NULL;
    Node* bestNode = queue->head;
    Node* bestPrev = NULL;
    int bestNum = -1;
    int bestDen = 1;
    while (current != NULL) {
        int waitTime = systemTime - current->process->arrival_time;
        int num = waitTime + current->process->burst_time;
        int den = current->process->burst_time;
        if ((num * bestDen) > (bestNum * den)) {
            bestNum = num;
            bestDen = den;
            bestNode = current;
            bestPrev = prev;
        }
        prev = current;
        current = current->next;
    }
    if (bestPrev == NULL) {
        queue->head = bestNode->next;
    } else {
        bestPrev->next = bestNode->next;
    }
    if (bestNode == queue->tail) {
        queue->tail = bestPrev;
    }
    PCB* out = bestNode->process;
    free(bestNode);
    return out;
}

PCB* pop_MLFQ() {
    for (int i = 0; i < 4; i++) {
        if (!empty(&mlfq[i])) {
            return pop(&mlfq[i]);
        }
    }
    return NULL;
}

void runOS(Algorithm algo) {
    printf("--- Starting OS Simulation ---\n");
    while (1) {
        PCB* new_process = NULL;
        if (system_time == 0) new_process = create_process(1, 0, 10);
        if (system_time == 1) new_process = create_process(2, 1, 8);
        if (system_time == 4) new_process = create_process(3, 4, 6);
        if (new_process != NULL) {
            if (algo == MLFQ) {
                push(new_process, &mlfq[0]);
            } else {
                push(new_process, &readyQueue);
            }
        }

        if (running_process == NULL) {
            print_queues(algo);
            if (algo == RR && !empty(&readyQueue)) {
                running_process = pop(&readyQueue);
            } else if (algo == HRRN && !empty(&readyQueue)) {
                running_process = pop_HRRN(&readyQueue, system_time);
            } else if (algo == MLFQ) {
                running_process = pop_MLFQ();
            }

            if (running_process != NULL) {
                running_process->state = RUNNING;
                if (algo == MLFQ) {
                    printf("Time %d: Process %d dispatched from MLFQ Level %d.\n",
                           system_time, running_process->pid, running_process->queue_level);
                } else {
                    printf("Time %d: Process %d dispatched to CPU.\n", system_time, running_process->pid);
                }
            }
        }

        if (running_process != NULL) {
            running_process->pc++;
            running_process->time_in_cpu++;

            if (running_process->pc >= running_process->burst_time) {
                printf("Time %d: Process %d FINISHED.\n", system_time, running_process->pid);
                running_process->state = FINISHED;
                free(running_process);
                running_process = NULL;
            }
            else {
                if (algo == RR && running_process->time_in_cpu == time_slice) {
                    printf("Time %d: Process %d preempted (RR Slice).\n", system_time, running_process->pid);
                    running_process->state = READY;
                    running_process->time_in_cpu = 0;
                    push(running_process, &readyQueue);
                    running_process = NULL;
                }
                else if (algo == MLFQ) {
                    int currentQuantum = 1 << running_process->queue_level;
                    if (running_process->time_in_cpu >= currentQuantum) {
                        printf("Time %d: Process %d preempted. ", system_time, running_process->pid);

                        if (running_process->queue_level < 3) {
                            running_process->queue_level++;
                            printf("Demoted to Level %d.\n", running_process->queue_level);
                        } else {
                            printf("Remaining in Level 3 (Round Robin).\n");
                        }

                        running_process->state = READY;
                        running_process->time_in_cpu = 0;
                        push(running_process, &mlfq[running_process->queue_level]);
                        running_process = NULL;
                    }
                }
            }
            print_queues(algo);
        }
        system_time++;
        if (system_time > 40) break;
    }
}