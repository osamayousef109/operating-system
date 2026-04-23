#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb.h"
#include "queues.h"
#include "scheduler.h"
#include "memory.h"
#include "../execution/interpreter.h"
#include "../execution/syscalls.h"

int system_time = 0;
PCB* running_process = NULL;
int current_slice_ticks = 0;
const int time_slice = 2;

Queue mlfq[4] = {
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL}
};

#define PROCESS_MEM_SIZE 20
int memory_slots[2] = {0, 0};

int allocate_memory_slot() {
    int slot_start = -1;
    if (memory_slots[0] == 0) { memory_slots[0] = 1; slot_start = 0; }
    else if (memory_slots[1] == 0) { memory_slots[1] = 1; slot_start = 20; }

    if (slot_start != -1) {
        for (int i = slot_start; i < slot_start + PROCESS_MEM_SIZE; i++) {
            main_memory[i].name[0] = '\0';
            main_memory[i].value[0] = '\0';
        }
    }
    return slot_start;
}

void free_memory_slot(int lower_boundary) {
    if (lower_boundary == 0) memory_slots[0] = 0;
    else if (lower_boundary == 20) memory_slots[1] = 0;

    for(int i = lower_boundary; i < lower_boundary + PROCESS_MEM_SIZE; i++) {
        memset(main_memory[i].name, 0, 20);
        memset(main_memory[i].value, 0, 50);
    }
}
void sync_pcb_to_memory(PCB* p) {
    if (p->mem_lower_boundary == -1) return;
    int base = p->mem_lower_boundary + 9;

    sprintf(main_memory[base + 0].name, "pid");     sprintf(main_memory[base + 0].value, "%d", p->pid);
    sprintf(main_memory[base + 1].name, "state");   sprintf(main_memory[base + 1].value, "%d", p->state);
    sprintf(main_memory[base + 2].name, "pc");      sprintf(main_memory[base + 2].value, "%d", p->pc);
    sprintf(main_memory[base + 3].name, "mem_low"); sprintf(main_memory[base + 3].value, "%d", p->mem_lower_boundary);
    sprintf(main_memory[base + 4].name, "file");    strncpy(main_memory[base + 4].value, p->fileName, 49);
    sprintf(main_memory[base + 5].name, "queue_L"); sprintf(main_memory[base + 5].value, "%d", p->queue_level);
    sprintf(main_memory[base + 6].name, "burst");   sprintf(main_memory[base + 6].value, "%d", p->burst_time);
    sprintf(main_memory[base + 7].name, "arrival"); sprintf(main_memory[base + 7].value, "%d", p->arrival_time);
}
void load_program_to_memory(PCB* p) {
    int lineCount = 0;
    char** lines = readLines(p->fileName, &lineCount);

    if (lines == NULL) {
        printf("OS Error: Could not find %s on disk.\n", p->fileName);
        return;
    }
    for (int i = 0; i < lineCount && i < 9; i++) {
        sprintf(main_memory[p->mem_lower_boundary + i].name, "c%d", i);
        strncpy(main_memory[p->mem_lower_boundary + i].value, lines[i], 49);
        free(lines[i]);
    }
    free(lines);
    for (int i = lineCount; i < 9; i++) {
        main_memory[p->mem_lower_boundary + i].name[0] = '\0';
        main_memory[p->mem_lower_boundary + i].value[0] = '\0';
    }
    sync_pcb_to_memory(p);

    printf("Time %d: Process %d loaded into memory at word %d.\n", system_time, p->pid, p->mem_lower_boundary);
}


PCB* create_process(int id, int arrival, int burst, const char* filename) {
    PCB* p = (PCB*)malloc(sizeof(PCB));
    p->pid = id;
    p->state = READY;
    p->pc = 0;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->queue_level = 0;
    strncpy(p->fileName, filename, 49);
    p->fileName[49] = '\0';

    int mem_start = allocate_memory_slot();

    if (mem_start != -1) {
        p->mem_lower_boundary = mem_start;
        load_program_to_memory(p);
    } else {
        printf("Time %d: Memory full. Process %d (%s) stored in Disk Queue.\n", system_time, p->pid, p->fileName);
        p->mem_lower_boundary = -1;
    }
    return p;
}
PCB* pop_HRRN(Queue* queue, int currentTime) {
    if (empty(queue)) return NULL;
    Node* current = queue->head;
    Node* prev = NULL;
    Node* bestNode = queue->head;
    Node* bestPrev = NULL;

    float highest_rr = -1.0;

    while (current != NULL) {
        int waitTime = currentTime - current->process->arrival_time;
        float responseRatio = (float)(waitTime + current->process->burst_time) / (float)current->process->burst_time;

        if (responseRatio > highest_rr) {
            highest_rr = responseRatio;
            bestNode = current;
            bestPrev = prev;
        }
        prev = current;
        current = current->next;
    }

    if (bestPrev == NULL) queue->head = bestNode->next;
    else bestPrev->next = bestNode->next;
    if (bestNode == queue->tail) queue->tail = bestPrev;

    PCB* out = bestNode->process;
    free(bestNode);
    return out;
}

PCB* pop_MLFQ() {
    for (int i = 0; i < 4; i++) {
        if (!empty(&mlfq[i])) return pop(&mlfq[i]);
    }
    return NULL;
}

void runOS(Algorithm algo) {
    printf("--- Starting OS Simulation ---\n");

    while (1) {
        if (algo == MLFQ) {
            while (!empty(&readyQueue)) {
                PCB* unblocked_proc = pop(&readyQueue);
                push(unblocked_proc, &mlfq[unblocked_proc->queue_level]);
            }
        }

        PCB* new_process = NULL;
        if (system_time == 0) new_process = create_process(1, 0, 7, "program1.txt");
        if (system_time == 1) new_process = create_process(2, 1, 7, "program2.txt");
        if (system_time == 4) new_process = create_process(3, 4, 9, "program3.txt");

        if (new_process != NULL) {
            if (new_process->mem_lower_boundary == -1) {
                push(new_process, &diskQueue);
            } else {
                if (algo == MLFQ) push(new_process, &mlfq[0]);
                else push(new_process, &readyQueue);
            }
        }

        if (running_process == NULL) {
            if (algo == RR && !empty(&readyQueue)) {
                running_process = pop(&readyQueue);
            } else if (algo == HRRN && !empty(&readyQueue)) {
                running_process = pop_HRRN(&readyQueue, system_time);
            } else if (algo == MLFQ) {
                running_process = pop_MLFQ();
            }

            if (running_process != NULL) {
                running_process->state = RUNNING;
                sync_pcb_to_memory(running_process);
                current_slice_ticks = 0;
                printf("Time %d: Process %d dispatched to CPU.\n", system_time, running_process->pid);
            }
        }
        if (running_process != NULL) {
            current_slice_ticks++;

            execute_one_instruction();
            sync_pcb_to_memory(running_process);

            if (running_process->state == BLOCKED) {
                printf("Time %d: Process %d BLOCKED by Semaphore.\n", system_time, running_process->pid);
                running_process = NULL;
            }
            else if (running_process->pc >= 9 || main_memory[running_process->mem_lower_boundary + running_process->pc - 1].value[0] == '\0') {
                printf("Time %d: Process %d FINISHED.\n", system_time, running_process->pid);

                free_memory_slot(running_process->mem_lower_boundary);
                free(running_process);
                running_process = NULL;

                if (!empty(&diskQueue)) {
                    PCB* disk_proc = pop(&diskQueue);
                    disk_proc->mem_lower_boundary = allocate_memory_slot();
                    load_program_to_memory(disk_proc);

                    if (algo == MLFQ) push(disk_proc, &mlfq[0]);
                    else push(disk_proc, &readyQueue);
                }
            }
            else {
                if (algo == RR && current_slice_ticks == time_slice) {
                    printf("Time %d: Process %d preempted (RR Slice).\n", system_time, running_process->pid);
                    running_process->state = READY;
                    sync_pcb_to_memory(running_process);
                    push(running_process, &readyQueue);
                    running_process = NULL;
                }
                else if (algo == MLFQ) {
                    int currentQuantum = 1 << running_process->queue_level;
                    if (current_slice_ticks >= currentQuantum) {
                        printf("Time %d: Process %d preempted.\n", system_time, running_process->pid);
                        if (running_process->queue_level < 3) running_process->queue_level++;

                        running_process->state = READY;
                        sync_pcb_to_memory(running_process);
                        push(running_process, &mlfq[running_process->queue_level]);
                        running_process = NULL;
                    }
                }
            }
        }

        system_time++;
        if (system_time > 60) break;
    }
}