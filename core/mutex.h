
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

void semWait(mutex* mutex);
void semSignal(mutex* mutex);

#endif //MUTEX_H
