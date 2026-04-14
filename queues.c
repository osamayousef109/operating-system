#include <stdio.h>
#include <stdlib.h>
#include "pcb.h"
#include "queues.h"

Queue readyQueue = {NULL,NULL};
Queue blockedQueue = {NULL,NULL};

int empty(Queue* queue) {
    return queue->tail == NULL;
}

void push(PCB* process, Queue* queue) {
    Node* node = (Node*) malloc(sizeof(Node));
    node->process = process;
    node->next = NULL;
    if (empty(queue)) {
        queue->head = node;
        queue->tail = node;
    }else {
        queue->tail->next = node;
        queue->tail = node;
    }
}

PCB* pop(Queue* queue) {
    if (empty(queue)) return NULL;
    Node* tempNode = queue->head;
    PCB* processToRun = tempNode->process;
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    free(tempNode);
    return processToRun;
}

void print_queue(Queue* queue, const char* name) {
    printf("%s: [", name);

    if (empty(queue)) {
        printf("]\n");
        return;
    }

    Node* current = queue->head;
    while (current != NULL) {
        printf("P%d", current->process->pid);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}