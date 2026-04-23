#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../scheduler/scheduler.h"
#include "mutex.h"
#include "memory.h"
#include "syscalls.h"
#include "interpreter.h"

mutex* getMutexByName(char* name) {
    if (name == NULL) return NULL;
    if (strcmp(name, "userInput") == 0) return &system_mutexes[userInput];
    if (strcmp(name, "file") == 0) return &system_mutexes[file];
    if (strcmp(name, "userOutput") == 0) return &system_mutexes[userOutput];
    return NULL;
}

char* getValue(char* token) {
    if (token == NULL) return NULL;

    int var_start_idx = running_process->mem_lower_boundary + 17;

    for (int i = 0; i < VARIABLES_PER_PROCESS; i++) {
        int idx = var_start_idx + i;
        if (strcmp(main_memory[idx].name, token) == 0) {
            return main_memory[idx].value;
        }
    }
    return token;
}

void updateMemory(char* name, char* value) {
    int var_start_idx = running_process->mem_lower_boundary + 17;

    for (int i = 0; i < VARIABLES_PER_PROCESS; i++) {
        int idx = var_start_idx + i;
        if (strcmp(main_memory[idx].name, name) == 0) {
            strncpy(main_memory[idx].value, value, 49);
            main_memory[idx].value[49] = '\0';
            return;
        }
    }

    for (int i = 0; i < VARIABLES_PER_PROCESS; i++) {
        int idx = var_start_idx + i;
        if (main_memory[idx].name[0] == '\0') {
            strncpy(main_memory[idx].name, name, 19);
            main_memory[idx].name[19] = '\0';

            strncpy(main_memory[idx].value, value, 49);
            main_memory[idx].value[49] = '\0';
            return;
        }
    }

    printf("Memory Error: No variable space left for PID %d\n", running_process->pid);
}

void execute_one_instruction() {
    if (running_process == NULL) return;

    if (running_process->pc >= 9) {
        return;
    }

    int mem_idx = running_process->mem_lower_boundary + running_process->pc;
    char* instruction = main_memory[mem_idx].value;

    if (instruction == NULL || instruction[0] == '\0') {
        running_process->pc = 9;
        return;
    }

    running_process->pc++;

    char line_copy[100];
    strncpy(line_copy, instruction, 99);
    line_copy[99] = '\0';

    char* cmd  = strtok(line_copy, " \r\n\t");
    char* arg1 = strtok(NULL, " \r\n\t");
    char* arg2 = strtok(NULL, " \r\n\t");
    char* arg3 = strtok(NULL, " \r\n\t");

    if (cmd == NULL) return;

    if (strcmp(cmd, "print") == 0) {
        char* val = getValue(arg1);
        print(val);
        printf("\n");
    }
    else if (strcmp(cmd, "assign") == 0) {
        if (arg2 != NULL && strcmp(arg2, "input") == 0) {
            char* val = input();
            updateMemory(arg1, val);
            free(val);
        }
        else if (arg2 != NULL && strcmp(arg2, "readFile") == 0) {
            char* fileToRead = getValue(arg3);
            char* content = readFile(fileToRead);
            if (content != NULL) {
                updateMemory(arg1, content);
                free(content);
            }
        }
        else {
            updateMemory(arg1, getValue(arg2));
        }
    }
    else if (strcmp(cmd, "writeFile") == 0) {
        char* file_target = getValue(arg1);
        char* data = getValue(arg2);
        writeFile(file_target, data);
    }
    else if (strcmp(cmd, "readFile") == 0) {
        char* fileToRead = getValue(arg1);
        char* content = readFile(fileToRead);
        if (content) free(content);
    }
    else if (strcmp(cmd, "printFromTo") == 0) {
        int start = atoi(getValue(arg1));
        int end = atoi(getValue(arg2));
        for (int j = start; j <= end; j++) {
            printf("%d\n", j);
        }
    }
    else if (strcmp(cmd, "semWait") == 0) {
        mutex* target_mutex = getMutexByName(arg1);
        if (target_mutex != NULL) {
            semWait(target_mutex);
        } else {
            printf("OS Error: Unknown mutex '%s'\n", arg1);
        }
    }
    else if (strcmp(cmd, "semSignal") == 0) {
        mutex* target_mutex = getMutexByName(arg1);
        if (target_mutex != NULL) {
            semSignal(target_mutex);
        } else {
            printf("OS Error: Unknown mutex '%s'\n", arg1);
        }
    }
    else {
        printf("Syntax Error at PC %d: Unknown command '%s'\n", running_process->pc - 1, cmd);
    }
}