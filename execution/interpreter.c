#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"

void loadProgram(char* filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    char line[100]; 

    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        parseAndExecute(line);
    }

    fclose(file);
}

void parseAndExecute(char* instruction) {
    char command[100] = "";
    int cur = 0;
    for (int i = 0; instruction[i] != ' ' && instruction[i] != '\0'; i++) {
        command[i] = instruction[i];
    }
    if (strcmp(command, "print") == 0) {
        char *message = instruction + strlen(command) + 1;
        printf("%s\n", message);
    }
    else if (strcmp(command, "assign") == 0) {
        int x = 0;
    }
    else if (strcmp(command, "writeFile") == 0) 
    {
        int x = 0;
    }
    else if (strcmp(command, "readFile") == 0) 
    {
        int x = 0;
    }
    else if (strcmp(command, "printFromTo") == 0){
        printf("Unknown command: %s\n", command);
    }

}

int main() {
    loadProgram("chickenbicken.txt");
    return 0;
}
