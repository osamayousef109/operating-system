//
// Created by MSi on 4/14/2026.
//

#ifndef QUEUES_H
#define QUEUES_H

#include "pcb.h"

typedef struct Node {
    PCB* process;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
} Queue;

extern Queue readyQueue;
extern Queue blockedQueue;
extern Queue diskQueue;

int empty(Queue* queue);
void push(PCB* process, Queue* queue);
PCB* pop(Queue* queue);
void print_queue(Queue* queue, const char* name);
PCB* removeProcessByPID(Queue* queue, int pid);
#endif
