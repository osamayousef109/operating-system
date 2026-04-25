//
// Created by MSi on 4/14/2026.
//
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "../core/pcb.h"
typedef enum {RR, HRRN, MLFQ} Algorithm;

extern int system_time;
extern PCB* running_process;

void runOS(Algorithm algo);

#endif //SCHEDULER_H
