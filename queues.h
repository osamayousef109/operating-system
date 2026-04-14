//
// Created by MSi on 4/14/2026.
//

#ifndef QUEUES_H
#define QUEUES_H

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

int empty(Queue* queue);
void push(PCB* process, Queue* queue);
PCB* pop(Queue* queue);
void print_queue(Queue* queue, const char* name);
#endif
