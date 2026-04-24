#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb.h"
#include "queues.h"
#include "scheduler.h"
#include "memory.h"
#include "../execution/syscalls.h"
#include "../execution/interpreter.h"

int RR_TIME_SLICE = 2;

int p1_arrival = 0;
int p2_arrival = 1;
int p3_arrival = 4;

int system_time = 0;
PCB* running_process = NULL;
int current_slice_ticks = 0;

Queue mlfq[4] = {
    {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}
};

#define PROCESS_MEM_SIZE 20
int memory_slots[2] = {0, 0};

int allocate_memory_slot() {
    if (memory_slots[0] == 0) { memory_slots[0] = 1; return 0; }
    if (memory_slots[1] == 0) { memory_slots[1] = 1; return 20; }
    return -1;
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
    // --- CRITICAL FIX: Guard against buffer underflow ---
    if (p->mem_lower_boundary == -1) {
        return;
    }
    // ----------------------------------------------------

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
}

int swap_out_victim(Algorithm algo) {
    PCB* victim = NULL;
    Node* curr = blockedQueue.head;

    // Check Blocked first
    while (curr != NULL) {
        if (curr->process->mem_lower_boundary != -1) { victim = curr->process; break; }
        curr = curr->next;
    }

    // Check MLFQ (L3 to L0) or ReadyQueue
    if (victim == NULL) {
        if (algo == MLFQ) {
            for (int i = 3; i >= 0; i--) {
                curr = mlfq[i].head;
                while (curr != NULL) {
                    if (curr->process->mem_lower_boundary != -1) { victim = curr->process; break; }
                    curr = curr->next;
                }
                if (victim != NULL) break;
            }
        } else {
            curr = readyQueue.head;
            while (curr != NULL) {
                if (curr->process->mem_lower_boundary != -1) { victim = curr->process; break; }
                curr = curr->next;
            }
        }
    }

    if (victim != NULL) {
        printf("Time %d: Swapping OUT Process %d to disk to free memory.\n", system_time, victim->pid);
        int freed_slot = victim->mem_lower_boundary;

        // --- REAL FILE I/O SWAPPING ---
        char swap_filename[50];
        sprintf(swap_filename, "swap_%d.txt", victim->pid);
        FILE* swap_file = fopen(swap_filename, "w");

        if (swap_file != NULL) {
            for (int i = 0; i < 20; i++) {
                fprintf(swap_file, "%s\n%s\n", main_memory[freed_slot + i].name, main_memory[freed_slot + i].value);
            }
            fclose(swap_file);
        } else {
            printf("OS FATAL ERROR: Failed to create physical swap file for PID %d\n", victim->pid);
        }
        // ------------------------------

        victim->mem_lower_boundary = -1;
        free_memory_slot(freed_slot);
        return freed_slot;
    }

    return -1;
}

PCB* create_process(int id, int arrival, int burst, const char* filename, Algorithm algo) {
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
    if (mem_start == -1) {
        mem_start = swap_out_victim(algo);
        if (mem_start == 0) memory_slots[0] = 1;
        else if (mem_start == 20) memory_slots[1] = 1;
    }

    p->mem_lower_boundary = mem_start;

    // --- CRITICAL FIX: Only load if we successfully secured a memory slot ---
    if (mem_start != -1) {
        load_program_to_memory(p);
        printf("Time %d: Process %d loaded into Memory.\n", system_time, p->pid);
    } else {
        printf("Time %d: Process %d forced to disk immediately (Memory Full).\n", system_time, p->pid);
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

void print_queues(Algorithm algo) {
    printf("\n--- System Queues Status [Time %d] ---\n", system_time);
    if (algo == MLFQ) {
        for (int i = 0; i < 4; i++) {
            char name[30];
            sprintf(name, "MLFQ Level %d (Quantum %d)", i, (1 << i));
            print_queue(&mlfq[i], name);
        }
    } else {
        print_queue(&readyQueue, "Ready Queue");
    }
    print_queue(&blockedQueue, "Blocked Queue");
    printf("--------------------------------------\n\n");
}

int stepOS(Algorithm algo) {
    int work_left = (running_process != NULL || !empty(&readyQueue) || !empty(&blockedQueue));
    if (algo == MLFQ) {
        for(int i=0; i<4; i++) if(!empty(&mlfq[i])) work_left = 1;
    }
    if (system_time > 4 && !work_left) return 0;

    if (algo == MLFQ) {
        while (!empty(&readyQueue)) {
            PCB* unblocked_proc = pop(&readyQueue);
            push(unblocked_proc, &mlfq[unblocked_proc->queue_level]);
        }
    }

    PCB* new_process = NULL;
    if (system_time == p1_arrival) new_process = create_process(1, p1_arrival, 10, "program1.txt", algo);
    if (system_time == p2_arrival) new_process = create_process(2, p2_arrival, 8, "program2.txt", algo);
    if (system_time == p3_arrival) new_process = create_process(3, p3_arrival, 6, "program3.txt", algo);

    if (new_process != NULL) {
        if (algo == MLFQ) push(new_process, &mlfq[0]);
        else push(new_process, &readyQueue);
    }

    if (running_process != NULL) {
        int preempted = 0;

        if (algo == MLFQ) {
            for (int i = 0; i < running_process->queue_level; i++) {
                if (!empty(&mlfq[i])) {
                    printf("Time %d: Process %d preempted by HIGHER priority.\n", system_time, running_process->pid);
                    running_process->state = READY;
                    sync_pcb_to_memory(running_process);
                    push(running_process, &mlfq[running_process->queue_level]);
                    running_process = NULL;
                    preempted = 1;
                    print_queues(algo);
                    break;
                }
            }
        }

        if (!preempted) {
            if (algo == RR && current_slice_ticks >= RR_TIME_SLICE) {
                printf("Time %d: Process %d preempted (RR Slice).\n", system_time, running_process->pid);
                running_process->state = READY;
                sync_pcb_to_memory(running_process);
                push(running_process, &readyQueue);
                running_process = NULL;
                print_queues(algo);
            }
            else if (algo == MLFQ) {
                int currentQuantum = 1 << running_process->queue_level;
                if (current_slice_ticks >= currentQuantum) {
                    printf("Time %d: Process %d preempted (Quantum over).\n", system_time, running_process->pid);
                    if (running_process->queue_level < 3) running_process->queue_level++;

                    running_process->state = READY;
                    sync_pcb_to_memory(running_process);
                    push(running_process, &mlfq[running_process->queue_level]);
                    running_process = NULL;
                    print_queues(algo);
                }
            }
        }
    }

    if (running_process == NULL) {
        if (algo == RR && !empty(&readyQueue)) running_process = pop(&readyQueue);
        else if (algo == HRRN && !empty(&readyQueue)) running_process = pop_HRRN(&readyQueue, system_time);
        else if (algo == MLFQ) running_process = pop_MLFQ();

        if (running_process != NULL) {
            if (running_process->mem_lower_boundary == -1) {
                int new_slot = allocate_memory_slot();
                if (new_slot == -1) {
                    new_slot = swap_out_victim(algo);
                    if (new_slot == 0) memory_slots[0] = 1;
                    else if (new_slot == 20) memory_slots[1] = 1;
                }

                running_process->mem_lower_boundary = new_slot;

                // --- REAL FILE I/O SWAPPING IN ---
                char swap_filename[50];
                sprintf(swap_filename, "swap_%d.txt", running_process->pid);
                FILE* swap_file = fopen(swap_filename, "r");

                if (swap_file != NULL) {
                    for (int i = 0; i < 20; i++) {
                        char name[50] = {0};
                        char value[100] = {0};

                        if (fgets(name, sizeof(name), swap_file) != NULL) {
                            name[strcspn(name, "\n")] = '\0'; // Remove newline
                            strncpy(main_memory[new_slot + i].name, name, 19);
                        }
                        if (fgets(value, sizeof(value), swap_file) != NULL) {
                            value[strcspn(value, "\n")] = '\0';
                            strncpy(main_memory[new_slot + i].value, value, 49);
                        }
                    }
                    fclose(swap_file);
                    remove(swap_filename); // Disk space cleanup after loading back into RAM
                } else {
                    // Fallback to initial state if swap file goes missing
                    load_program_to_memory(running_process);
                }
                // ---------------------------------

                sync_pcb_to_memory(running_process);
                printf("Time %d: Process %d swapped IN from Disk to Memory.\n", system_time, running_process->pid);
            }

            running_process->state = RUNNING;
            sync_pcb_to_memory(running_process);
            current_slice_ticks = 0;
            printf("Time %d: Process %d dispatched to CPU.\n", system_time, running_process->pid);
            print_queues(algo);
        }
    }

    if (running_process != NULL) {
        current_slice_ticks++;
        execute_one_instruction();
        sync_pcb_to_memory(running_process);

        if (running_process->state == BLOCKED) {
            printf("Time %d: Process %d BLOCKED by Semaphore.\n", system_time, running_process->pid);
            running_process = NULL;
            print_queues(algo);
        }
        else if (running_process->pc >= 9 || main_memory[running_process->mem_lower_boundary + running_process->pc - 1].value[0] == '\0') {
            printf("Time %d: Process %d FINISHED.\n", system_time, running_process->pid);

            free_memory_slot(running_process->mem_lower_boundary);
            free(running_process);
            running_process = NULL;
            print_queues(algo);
        }
    }

    printMemory();
    system_time++;
    return 1;
}