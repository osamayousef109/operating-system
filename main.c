#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "raylib.h"
#include "../scheduler/scheduler.h"
#include "mutex.h"
#include "memory.h"
#include "queues.h"

extern int system_time;
extern PCB* running_process;
extern Queue readyQueue, blockedQueue, mlfq[4];
extern MemoryWord main_memory[40];
extern int current_slice_ticks;
extern int RR_TIME_SLICE;

int gui_input_state = 0;
char gui_input_buffer[100] = {0};
extern char gui_input_var[50];
extern int gui_input_pid;
extern char gui_current_instruction[100];

char gui_output_log[10][200] = {0};
int log_count = 0;

void gui_print(const char* text) {
    if (text == NULL) return;
    if (log_count < 10) {
        strncpy(gui_output_log[log_count], text, 199);
        log_count++;
    } else {
        for (int i = 1; i < 10; i++) strcpy(gui_output_log[i-1], gui_output_log[i]);
        strncpy(gui_output_log[9], text, 199);
    }
}

void DrawQueueGUI(Queue* q, int startX, int startY, const char* label, Color color, Algorithm algo, bool isReady) {
    DrawText(label, startX, startY, 20, DARKGRAY);
    int currentX = startX + MeasureText(label, 20) + 10;

    if (empty(q)) {
        DrawText("[ Empty ]", currentX, startY, 20, LIGHTGRAY);
        return;
    }

    Node* current = q->head;
    while (current != NULL) {
        DrawRectangle(currentX, startY - 2, 40, 24, color);
        DrawRectangleLines(currentX, startY - 2, 40, 24, DARKGRAY);
        DrawText(TextFormat("P%d", current->process->pid), currentX + 8, startY + 2, 20, WHITE);

        if (algo == HRRN && isReady) {
            int waitTime = system_time - current->process->arrival_time;
            float rr = (float)(waitTime + current->process->burst_time) / (float)current->process->burst_time;
            DrawText(TextFormat("RR:%.1f", rr), currentX - 2, startY + 26, 10, DARKBLUE);
        }

        currentX += 50;
        if (current->next != NULL) {
            DrawText("->", currentX, startY + 2, 20, DARKGRAY);
            currentX += 30;
        }
        current = current->next;
    }
}

void DrawSwappedOutGUI(int startX, int startY, const char* label, Color color) {
    DrawText(label, startX, startY, 20, DARKGRAY);
    int currentX = startX + MeasureText(label, 20) + 10;
    bool found = false;

    Node* curr = readyQueue.head;
    while (curr != NULL) {
        if (curr->process->mem_lower_boundary == -1) {
            DrawRectangle(currentX, startY - 2, 40, 24, color);
            DrawRectangleLines(currentX, startY - 2, 40, 24, DARKGRAY);
            DrawText(TextFormat("P%d", curr->process->pid), currentX + 8, startY + 2, 20, WHITE);
            currentX += 50; found = true;
        }
        curr = curr->next;
    }
    for (int i = 0; i < 4; i++) {
        curr = mlfq[i].head;
        while (curr != NULL) {
            if (curr->process->mem_lower_boundary == -1) {
                DrawRectangle(currentX, startY - 2, 40, 24, color);
                DrawRectangleLines(currentX, startY - 2, 40, 24, DARKGRAY);
                DrawText(TextFormat("P%d", curr->process->pid), currentX + 8, startY + 2, 20, WHITE);
                currentX += 50; found = true;
            }
            curr = curr->next;
        }
    }
    curr = blockedQueue.head;
    while (curr != NULL) {
        if (curr->process->mem_lower_boundary == -1) {
            DrawRectangle(currentX, startY - 2, 40, 24, color);
            DrawRectangleLines(currentX, startY - 2, 40, 24, DARKGRAY);
            DrawText(TextFormat("P%d", curr->process->pid), currentX + 8, startY + 2, 20, WHITE);
            currentX += 50; found = true;
        }
        curr = curr->next;
    }

    if (!found) DrawText("[ Empty ]", currentX, startY, 20, LIGHTGRAY);
}

int main() {
    init_mutexes();
    const int screenWidth = 1200;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "GUC OS Simulator");
    SetTargetFPS(60);

    bool osFinished = false;
    bool autoPlay = false;
    int framesCounter = 0;
    Algorithm currentAlgo = RR;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_A)) autoPlay = !autoPlay;
        if (gui_input_state == 1) {
            autoPlay = false;
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (strlen(gui_input_buffer) < 49)) {
                    int len = strlen(gui_input_buffer);
                    gui_input_buffer[len] = (char)key;
                    gui_input_buffer[len+1] = '\0';
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = strlen(gui_input_buffer);
                if (len > 0) gui_input_buffer[len-1] = '\0';
            }
            if (IsKeyPressed(KEY_ENTER) && strlen(gui_input_buffer) > 0) {
                char echo[120];
                sprintf(echo, "> %s", gui_input_buffer);
                gui_print(echo);
                gui_input_state = 2;
            }
        }

        bool timeToStep = (IsKeyPressed(KEY_SPACE) || (autoPlay && framesCounter % 30 == 0));

        if (timeToStep && !osFinished && gui_input_state != 1) {
            int status = stepOS(currentAlgo);
            if (status == 0) osFinished = true;
        }
        framesCounter++;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(0, 0, screenWidth, 60, DARKBLUE);
        DrawText("OS SCHEDULER & MEMORY VISUALIZER", 20, 15, 30, WHITE);

        DrawText("CONTROLS:", 20, 70, 20, BLACK);
        DrawText("[SPACE] Step Forward", 140, 70, 20, DARKGRAY);
        DrawText("[A] Toggle Auto-Play", 380, 70, 20, autoPlay ? GREEN : DARKGRAY);

        if (osFinished) DrawText("STATUS: FINISHED", 800, 70, 20, RED);
        else DrawText(TextFormat("SYSTEM TIME: %d", system_time), 800, 70, 20, DARKBLUE);

        DrawRectangle(20, 110, 450, 110, LIGHTGRAY);
        DrawRectangleLines(20, 110, 450, 110, DARKGRAY);

        DrawText("CPU (Running)", 30, 120, 20, BLACK);
        if (running_process != NULL) {
            if (currentAlgo == RR) DrawText(TextFormat("RR Ticks: %d / %d", current_slice_ticks, RR_TIME_SLICE), 270, 125, 15, DARKBLUE);
            else if (currentAlgo == MLFQ) DrawText(TextFormat("MLFQ Ticks: %d / %d", current_slice_ticks, (1 << running_process->queue_level)), 250, 125, 15, DARKBLUE);
            else if (currentAlgo == HRRN) DrawText(TextFormat("Ticks Elapsed: %d", current_slice_ticks), 270, 125, 15, DARKBLUE);

            DrawText(TextFormat("PID: %d", running_process->pid), 30, 150, 20, DARKBLUE);
            DrawText("State: RUNNING", 130, 150, 20, GREEN);

            DrawText(TextFormat("PC: %d", running_process->pc), 330, 150, 20, BLACK);
            DrawText(TextFormat("Cmd: %s", gui_current_instruction), 30, 180, 20, MAROON);
        } else {
            DrawText("IDLE", 200, 150, 30, GRAY);
        }

        int qY = 240;
        DrawText("SYSTEM QUEUES", 30, qY, 20, BLACK);
        DrawLine(20, qY + 25, 470, qY + 25, DARKGRAY);

        if (currentAlgo == MLFQ) {
            DrawQueueGUI(&mlfq[0], 30, qY + 40, "MLFQ L0:", BLUE, currentAlgo, true);
            DrawQueueGUI(&mlfq[1], 30, qY + 80, "MLFQ L1:", SKYBLUE, currentAlgo, true);
            DrawQueueGUI(&mlfq[2], 30, qY + 120, "MLFQ L2:", DARKBLUE, currentAlgo, true);
            DrawQueueGUI(&mlfq[3], 30, qY + 160, "MLFQ L3:", PURPLE, currentAlgo, true);
        } else {
            DrawQueueGUI(&readyQueue, 30, qY + 40, "Ready Queue:", BLUE, currentAlgo, true);
        }
        DrawQueueGUI(&blockedQueue, 30, qY + 210, "Blocked Queue:", RED, currentAlgo, false);

        DrawSwappedOutGUI(30, qY + 250, "Disk (Swapped):", ORANGE);

        DrawRectangle(20, 520, 450, 190, BLACK);
        DrawRectangleLines(20, 520, 450, 190, DARKGRAY);
        DrawText("TERMINAL OUTPUT", 30, 530, 20, GREEN);

        for (int i = 0; i < 10; i++) {
            if (gui_output_log[i][0] != '\0') {
                DrawText(gui_output_log[i], 30, 560 + (i * 14), 15, LIGHTGRAY);
            }
        }

        if (gui_input_state == 1) {
            DrawRectangle(20, 720, 450, 60, DARKGREEN);
            DrawText(TextFormat("Process %d needs a value for '%s':", gui_input_pid, gui_input_var), 30, 730, 15, LIGHTGRAY);
            DrawText(TextFormat("> %s_", gui_input_buffer), 30, 755, 15, WHITE);
            DrawText("(Press ENTER)", 340, 755, 15, GRAY);
        }

        DrawRectangle(500, 110, 680, 670, Fade(SKYBLUE, 0.2f));
        DrawRectangleLines(500, 110, 680, 670, DARKBLUE);
        DrawText("MAIN MEMORY (40 Words)", 520, 120, 20, DARKBLUE);

        int memStartY = 160;
        for (int i = 0; i < 20; i++) {
            int yPos = memStartY + (i * 30);
            DrawText(TextFormat("%02d |", i), 520, yPos, 15, DARKGRAY);
            if (main_memory[i].name[0] != '\0') {
                DrawText(main_memory[i].name, 560, yPos, 15, MAROON);
                DrawText("=", 660, yPos, 15, DARKGRAY);
                DrawText(main_memory[i].value, 680, yPos, 15, BLACK);
            } else {
                DrawText("[Empty]", 560, yPos, 15, GRAY);
            }

            int i2 = i + 20;
            DrawText(TextFormat("%02d |", i2), 850, yPos, 15, DARKGRAY);
            if (main_memory[i2].name[0] != '\0') {
                DrawText(main_memory[i2].name, 890, yPos, 15, MAROON);
                DrawText("=", 990, yPos, 15, DARKGRAY);
                DrawText(main_memory[i2].value, 1010, yPos, 15, BLACK);
            } else {
                DrawText("[Empty]", 890, yPos, 15, GRAY);
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}