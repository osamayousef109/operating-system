#ifndef PCB_H
#define PCB_H

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    FINISHED
} ProcessState;

typedef struct {
    int pid;
    ProcessState state;
    int pc;
    int mem_lower_boundary;
    int mem_upper_boundary;
    int arrival_time;
    int burst_time;
    int time_in_cpu;
    int queue_level;
} PCB;

#endif