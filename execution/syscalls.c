#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syscalls.h"

char* readFile(char* filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("\nFile Error: Could not read file '%s'. It may not exist.\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);

    if (bytesRead != fileSize) {
        perror("Error reading file or partial read");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[fileSize] = '\0';

    fclose(file);

    return buffer;
}

char** readLines(char* filename, int* outLineCount) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    int capacity = 10;
    int count = 0;

    char **lines = (char**)malloc(capacity * sizeof(char*));
    if (lines == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {

        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (count >= capacity) {
            capacity *= 2;
            char **temp = (char**)realloc(lines, capacity * sizeof(char*));
            if (temp == NULL) {
                perror("Error reallocating memory");
                for (int i = 0; i < count; i++) free(lines[i]);
                free(lines);
                fclose(file);
                return NULL;
            }
            lines = temp;
        }

        lines[count] = (char*)malloc(strlen(buffer) + 1);
        if (lines[count] != NULL) {
            strcpy(lines[count], buffer);
            count++;
        }
    }

    fclose(file);

    *outLineCount = count;

    return lines;
}

int writeFile(char* filename, char* content) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("\nFile Error: Could not create file '%s'.\n", filename);
        return 0;
    }


    if (fputs(content, file) == EOF) {
        perror("Error writing to file");
        fclose(file);
        return 0;
    }
    fflush(file);
    fclose(file);
    return 1;
}
void print(char* content) {
    printf(content);
}
char* input() {
    printf("Please enter a value\n");
    char* s = (char*) malloc(101);
    fgets(s, 101, stdin);
    s[strcspn(s, "\r\n")] = '\0';
    return s;
}

MemoryWord readMemory(int idx) {
    return main_memory[idx];
}
void writeMemory(int idx, char* name, char* value) {
    strncpy(main_memory[idx].name, name, 19);
    main_memory[idx].name[19] = '\0';

    strncpy(main_memory[idx].value, value, 49);
    main_memory[idx].value[49] = '\0';
}


