//
// Created by MSi on 4/14/2026.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef enum {RR, HRRN, MLFQ} Algorithm;

extern int system_time;

void runOS(Algorithm algo);

#endif //SCHEDULER_H
