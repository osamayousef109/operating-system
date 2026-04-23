
#ifndef MUTEX_H
#define MUTEX_H
#include "queues.h"
typedef enum{
    userInput,
    userOutput,
    file
} mutex_type;

typedef struct {
    mutex_type type;
    int available;
    Queue blocked_queue;
} mutex;

extern mutex system_mutexes[3];

void semWait(mutex* mutex);
void semSignal(mutex* mutex);
void init_mutexes();

#endif //MUTEX_H
