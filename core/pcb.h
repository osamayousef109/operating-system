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
    char fileName[50];
    int queue_level;
    int burst_time;
    int arrival_time;
} PCB;

#endif