//
// Created by MSi on 4/14/2026.
//
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "pcb.h"
#include "queues.h"
typedef enum {RR, HRRN, MLFQ} Algorithm;
extern Queue mlfq[4];
extern int system_time;
extern PCB* running_process;

int stepOS(Algorithm algo);

#endif //SCHEDULER_H
