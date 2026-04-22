
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syscalls.h"

char* readFile(char* fileName) {
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    int capacity = 10;
    int length = 0;
    char* ret = malloc(capacity * sizeof(char));
    char ch;

    while ((ch = fgetc(file)) != EOF) {
        if (length + 1 >= capacity) {
            capacity *= 2;
            ret = realloc(ret, capacity * sizeof(char));
        }
        ret[length] = ch;
        length++;
    }

    ret[length] = '\0';
    return ret;
}

void writeFile(char* fileName, char* content) {

}
