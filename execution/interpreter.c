#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "interpreter.h"

/**
 * Utility: Check if string is a number (integer or string)
 */
int isNumber(const char* str) {
    if (str == NULL || str[0] == '\0') return 0;
    
    // Handle negative numbers
    int i = 0;
    if (str[0] == '-') i = 1;
    
    // Must have at least one digit
    if (i >= strlen(str)) return 0;
    
    for (; i < strlen(str); i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

/**
 * Get resource type from string name
 */
ResourceType getResourceTypeFromString(const char* resource_name) {
    if (strcmp(resource_name, "file") == 0) {
        return FILE_RESOURCE;
    } else if (strcmp(resource_name, "userInput") == 0) {
        return USER_INPUT_RESOURCE;
    } else if (strcmp(resource_name, "userOutput") == 0) {
        return USER_OUTPUT_RESOURCE;
    }
    return FILE_RESOURCE;  // Default
}

/**
 * Trim whitespace from string
 */
void trimWhitespace(char* str) {
    if (str == NULL) return;
    
    // Trim leading whitespace
    int start = 0;
    while (isspace(str[start])) start++;
    
    // Trim trailing whitespace
    int end = strlen(str) - 1;
    while (end > start && isspace(str[end])) end--;
    
    // Copy trimmed string
    int j = 0;
    for (int i = start; i <= end; i++) {
        str[j++] = str[i];
    }
    str[j] = '\0';
}

/**
 * Split instruction line into tokens
 * Returns number of tokens
 */
int tokenizeInstruction(const char* line, char tokens[10][MAX_VARIABLE_NAME]) {
    if (line == NULL) return 0;
    
    char lineCopy[MAX_INSTRUCTION_LENGTH];
    strcpy(lineCopy, line);
    
    int count = 0;
    char* token = strtok(lineCopy, " \t");
    
    while (token != NULL && count < 10) {
        strcpy(tokens[count], token);
        count++;
        token = strtok(NULL, " \t");
    }
    
    return count;
}

/**
 * Parse a single line into an Instruction
 */
Instruction parseInstruction(const char* line) {
    Instruction instr;
    instr.type = INSTR_INVALID;
    strcpy(instr.arg1, "");
    strcpy(instr.arg2, "");
    strcpy(instr.arg3, "");
    
    if (line == NULL || strlen(line) == 0) {
        instr.type = INSTR_END;
        return instr;
    }
    
    // Skip empty lines and comments
    if (line[0] == '#' || line[0] == ';') {
        instr.type = INSTR_END;
        return instr;
    }
    
    char tokens[10][MAX_VARIABLE_NAME];
    int token_count = tokenizeInstruction(line, tokens);
    
    if (token_count == 0) {
        instr.type = INSTR_END;
        return instr;
    }
    
    // Parse based on command
    if (strcmp(tokens[0], "print") == 0) {
        if (token_count >= 2) {
            instr.type = INSTR_PRINT;
            strcpy(instr.arg1, tokens[1]);
        }
    }
    else if (strcmp(tokens[0], "assign") == 0) {
        if (token_count >= 3) {
            instr.type = INSTR_ASSIGN;
            strcpy(instr.arg1, tokens[1]);  // Variable name
            strcpy(instr.arg2, tokens[2]);  // Value or "input"
        }
    }
    else if (strcmp(tokens[0], "writeFile") == 0) {
        if (token_count >= 3) {
            instr.type = INSTR_WRITE_FILE;
            strcpy(instr.arg1, tokens[1]);  // Filename
            strcpy(instr.arg2, tokens[2]);  // Data/variable name
        }
    }
    else if (strcmp(tokens[0], "readFile") == 0) {
        if (token_count >= 2) {
            instr.type = INSTR_READ_FILE;
            strcpy(instr.arg1, tokens[1]);  // Filename
        }
    }
    else if (strcmp(tokens[0], "printFromTo") == 0) {
        if (token_count >= 3) {
            instr.type = INSTR_PRINT_FROM_TO;
            strcpy(instr.arg1, tokens[1]);  // Start number/variable
            strcpy(instr.arg2, tokens[2]);  // End number/variable
        }
    }
    else if (strcmp(tokens[0], "semWait") == 0) {
        if (token_count >= 2) {
            instr.type = INSTR_SEM_WAIT;
            strcpy(instr.arg1, tokens[1]);  // Resource name
        }
    }
    else if (strcmp(tokens[0], "semSignal") == 0) {
        if (token_count >= 2) {
            instr.type = INSTR_SEM_SIGNAL;
            strcpy(instr.arg1, tokens[1]);  // Resource name
        }
    }
    
    return instr;
}

/**
 * Load and parse a program file
 */
Program* loadProgram(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("ERROR: Could not open program file: %s\n", filename);
        return NULL;
    }
    
    Program* program = (Program*)malloc(sizeof(Program));
    program->instructions = (Instruction*)malloc(MAX_PROGRAM_LINES * sizeof(Instruction));
    program->variables = (Variable*)malloc(MAX_VARIABLES_PER_PROCESS * sizeof(Variable));
    program->num_instructions = 0;
    program->num_variables = 0;
    program->pid = -1;
    
    // Read lines from file
    char line[MAX_INSTRUCTION_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL && program->num_instructions < MAX_PROGRAM_LINES) {
        // Remove newline
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        
        trimWhitespace(line);
        
        if (strlen(line) == 0 || line[0] == '#') {
            continue;  // Skip empty lines and comments
        }
        
        Instruction instr = parseInstruction(line);
        if (instr.type != INSTR_END) {
            program->instructions[program->num_instructions] = instr;
            program->num_instructions++;
        }
    }
    
    fclose(file);
    
    printf("Program loaded: %d instructions from %s\n", program->num_instructions, filename);
    return program;
}

/**
 * Get variable by name
 */
Variable* getVariable(Program* program, const char* var_name) {
    if (program == NULL || var_name == NULL) return NULL;
    
    for (int i = 0; i < program->num_variables; i++) {
        if (strcmp(program->variables[i].name, var_name) == 0) {
            return &program->variables[i];
        }
    }
    return NULL;
}

/**
 * Set variable value
 */
int setVariable(Program* program, const char* var_name, const char* value) {
    if (program == NULL || var_name == NULL || value == NULL) return -1;
    
    Variable* var = getVariable(program, var_name);
    if (var == NULL) return -1;
    
    strcpy(var->value, value);
    return 0;
}

/**
 * Create new variable
 */
int createVariable(Program* program, const char* var_name, const char* value) {
    if (program == NULL || var_name == NULL || value == NULL) return -1;
    
    // Check if variable already exists
    if (getVariable(program, var_name) != NULL) {
        return setVariable(program, var_name, value);
    }
    
    // Check if we have space
    if (program->num_variables >= MAX_VARIABLES_PER_PROCESS) {
        printf("ERROR: Maximum variables (%d) reached for process %d\n", 
               MAX_VARIABLES_PER_PROCESS, program->pid);
        return -1;
    }
    
    // Create new variable
    Variable* var = &program->variables[program->num_variables];
    strcpy(var->name, var_name);
    strcpy(var->value, value);
    program->num_variables++;
    
    return 0;
}

/**
 * Execute instruction and handle syscalls
 * External functions (syscalls) will be defined in syscalls.c
 */
extern void syscall_print(const char* value);
extern void syscall_input(char* buffer);
extern void syscall_writeFile(const char* filename, const char* data);
extern char* syscall_readFile(const char* filename);

/**
 * Execute the next instruction for a process
 */
int executeInstruction(InterpreterContext* context) {
    if (context == NULL || context->program == NULL) {
        return -1;
    }
    
    if (context->current_instruction >= context->program->num_instructions) {
        printf("Time: Process %d FINISHED all instructions.\n", context->process->pid);
        return 2;  // Process finished
    }
    
    Instruction* instr = &context->program->instructions[context->current_instruction];
    
    printf("Time: Process %d >> Executing: ", context->process->pid);
    
    // Execute instruction
    switch (instr->type) {
        case INSTR_PRINT: {
            printf("print %s\n", instr->arg1);
            Variable* var = getVariable(context->program, instr->arg1);
            if (var != NULL) {
                syscall_print(var->value);
            } else {
                printf("WARNING: Variable %s not found\n", instr->arg1);
            }
            break;
        }
        
        case INSTR_ASSIGN: {
            printf("assign %s %s\n", instr->arg1, instr->arg2);
            if (strcmp(instr->arg2, "input") == 0) {
                // Get input from user
                char input_buffer[MAX_VARIABLE_VALUE];
                syscall_input(input_buffer);
                createVariable(context->program, instr->arg1, input_buffer);
            } else {
                createVariable(context->program, instr->arg1, instr->arg2);
            }
            break;
        }
        
        case INSTR_WRITE_FILE: {
            printf("writeFile %s %s\n", instr->arg1, instr->arg2);
            Variable* var = getVariable(context->program, instr->arg2);
            const char* data = (var != NULL) ? var->value : instr->arg2;
            syscall_writeFile(instr->arg1, data);
            break;
        }
        
        case INSTR_READ_FILE: {
            printf("readFile %s\n", instr->arg1);
            char* file_data = syscall_readFile(instr->arg1);
            if (file_data != NULL) {
                // Store in a variable (use filename as variable)
                createVariable(context->program, instr->arg1, file_data);
                free(file_data);
            }
            break;
        }
        
        case INSTR_PRINT_FROM_TO: {
            printf("printFromTo %s %s\n", instr->arg1, instr->arg2);
            
            // Get start and end values
            int start = 0, end = 0;
            
            Variable* var1 = getVariable(context->program, instr->arg1);
            if (var1 != NULL) {
                start = atoi(var1->value);
            } else if (isNumber(instr->arg1)) {
                start = atoi(instr->arg1);
            }
            
            Variable* var2 = getVariable(context->program, instr->arg2);
            if (var2 != NULL) {
                end = atoi(var2->value);
            } else if (isNumber(instr->arg2)) {
                end = atoi(instr->arg2);
            }
            
            // Print numbers from start to end
            char buffer[10];
            for (int i = start; i <= end; i++) {
                snprintf(buffer, sizeof(buffer), "%d", i);
                syscall_print(buffer);
            }
            break;
        }
        
        case INSTR_SEM_WAIT: {
            printf("semWait %s\n", instr->arg1);
            ResourceType resource = getResourceTypeFromString(instr->arg1);
            int result = semWait(resource, context->process);
            if (result == 0) {
                // Process was blocked
                context->is_blocked = 1;
                context->waiting_resource = resource;
                return 1;  // Process blocked
            }
            break;
        }
        
        case INSTR_SEM_SIGNAL: {
            printf("semSignal %s\n", instr->arg1);
            ResourceType resource = getResourceTypeFromString(instr->arg1);
            semSignal(resource);
            break;
        }
        
        case INSTR_END:
            printf("END\n");
            return 2;  // Program finished
            
        case INSTR_INVALID:
            printf("INVALID\n");
            return -1;
            
        default:
            printf("UNKNOWN\n");
            return -1;
    }
    
    // Move program counter
    context->current_instruction++;
    return 0;  // Instruction executed successfully
}

/**
 * Free program resources
 */
void freeProgram(Program* program) {
    if (program == NULL) return;
    
    if (program->instructions != NULL) {
        free(program->instructions);
    }
    if (program->variables != NULL) {
        free(program->variables);
    }
    free(program);
}
