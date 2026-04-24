#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../scheduler/scheduler.h"
#include "mutex.h"
#include "memory.h"
#include "syscalls.h"

// --- GUI BRIDGE VARIABLES ---
extern int gui_input_state;
extern char gui_input_buffer[100];
extern void gui_print(const char* text);
extern int current_slice_ticks;
char gui_input_var[50] = {0};
int gui_input_pid = -1;
char gui_current_instruction[100] = {0};
// ----------------------------

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
    for (int i = 0; i < 3; i++) {
        int idx = var_start_idx + i;
        if (strcmp(main_memory[idx].name, token) == 0) return main_memory[idx].value;
    }
    return token;
}

void updateMemory(char* name, char* value) {
    if (name == NULL || value == NULL) return;
    int var_start_idx = running_process->mem_lower_boundary + 17;
    for (int i = 0; i < 3; i++) {
        int idx = var_start_idx + i;
        if (strcmp(main_memory[idx].name, name) == 0) {
            strncpy(main_memory[idx].value, value, 49);
            main_memory[idx].value[49] = '\0';
            return;
        }
    }
    for (int i = 0; i < 3; i++) {
        int idx = var_start_idx + i;
        if (main_memory[idx].name[0] == '\0') {
            strncpy(main_memory[idx].name, name, 19);
            main_memory[idx].name[19] = '\0';
            strncpy(main_memory[idx].value, value, 49);
            main_memory[idx].value[49] = '\0';
            return;
        }
    }
}

void execute_one_instruction() {
    if (running_process == NULL) return;

    while (running_process->pc < 9) {
        int mem_idx = running_process->mem_lower_boundary + running_process->pc;
        char* inst = main_memory[mem_idx].value;
        if (inst == NULL || inst[0] == '\0' || inst[0] == '\r' || inst[0] == '\n') {
            running_process->pc++;
        } else {
            break;
        }
    }

    if (running_process->pc >= 9) return;

    int mem_idx = running_process->mem_lower_boundary + running_process->pc;
    char* instruction = main_memory[mem_idx].value;

    strncpy(gui_current_instruction, instruction, 99);
    gui_current_instruction[99] = '\0';

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
        if (val != NULL) gui_print(val);
    }
    else if (strcmp(cmd, "assign") == 0) {
        if (arg2 != NULL && strcmp(arg2, "input") == 0) {
            if (gui_input_state == 0) {
                gui_input_state = 1;
                if (arg1 != NULL) strncpy(gui_input_var, arg1, 49);
                gui_input_pid = running_process->pid;

                current_slice_ticks--;
                running_process->pc--;
                return;
            } else if (gui_input_state == 2) {
                if (arg1 != NULL) updateMemory(arg1, gui_input_buffer);
                gui_input_state = 0;
                gui_input_buffer[0] = '\0';
            }
        }
        else if (arg2 != NULL && strcmp(arg2, "readFile") == 0) {
            char* fileToRead = getValue(arg3);
            if (fileToRead != NULL && arg1 != NULL) {
                char* content = readFile(fileToRead);
                if (content != NULL) {
                    updateMemory(arg1, content);
                    free(content);
                }
            }
        }
        else {
            if (arg1 != NULL && arg2 != NULL) updateMemory(arg1, getValue(arg2));
        }
    }
    else if (strcmp(cmd, "writeFile") == 0) {
        char* file_target = getValue(arg1);
        char* data = getValue(arg2);
        if (file_target != NULL && data != NULL) writeFile(file_target, data);
    }
    else if (strcmp(cmd, "readFile") == 0) {
        char* fileToRead = getValue(arg1);
        if (fileToRead != NULL) {
            char* content = readFile(fileToRead);
            if (content != NULL) free(content);
        }
    }
    else if (strcmp(cmd, "printFromTo") == 0) {
        char* v1 = getValue(arg1);
        char* v2 = getValue(arg2);

        if (v1 != NULL && v2 != NULL) {
            int start = atoi(v1);
            int end = atoi(v2);
            char buf[256] = {0};
            char temp[20];

            if (start <= end) {
                for (int j = start; j <= end; j++) {
                    if (strlen(buf) > 230) break;
                    sprintf(temp, "%d ", j);
                    strcat(buf, temp);
                }
            } else {
                for (int j = start; j >= end; j--) {
                    if (strlen(buf) > 230) break;
                    sprintf(temp, "%d ", j);
                    strcat(buf, temp);
                }
            }
            gui_print(buf);
        }
    }
    else if (strcmp(cmd, "semWait") == 0) {
        if (arg1 != NULL) {
            mutex* m = getMutexByName(arg1);
            if (m != NULL) semWait(m);
        }
    }
    else if (strcmp(cmd, "semSignal") == 0) {
        if (arg1 != NULL) {
            mutex* m = getMutexByName(arg1);
            if (m != NULL) semSignal(m);
        }
    }
}