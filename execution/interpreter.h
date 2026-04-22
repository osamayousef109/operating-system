#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "pcb.h"
#include "mutex.h"

// Maximum limits
#define MAX_INSTRUCTION_LENGTH 100
#define MAX_PROGRAM_LINES 50
#define MAX_VARIABLES_PER_PROCESS 3
#define MAX_VARIABLE_NAME 20
#define MAX_VARIABLE_VALUE 100

// Instruction types
typedef enum {
    INSTR_PRINT,        // print x
    INSTR_ASSIGN,       // assign x y (or assign x input)
    INSTR_WRITE_FILE,   // writeFile filename data
    INSTR_READ_FILE,    // readFile filename
    INSTR_PRINT_FROM_TO, // printFromTo x y
    INSTR_SEM_WAIT,     // semWait resource
    INSTR_SEM_SIGNAL,   // semSignal resource
    INSTR_INVALID,
    INSTR_END           // End of program
} InstructionType;

// Variable structure
typedef struct {
    char name[MAX_VARIABLE_NAME];
    char value[MAX_VARIABLE_VALUE];
} Variable;

// Instruction structure
typedef struct {
    InstructionType type;
    char arg1[MAX_VARIABLE_NAME];    // First argument (variable name or filename)
    char arg2[MAX_VARIABLE_NAME];    // Second argument (value or resource name)
    char arg3[MAX_VARIABLE_NAME];    // Third argument (for printFromTo)
} Instruction;

// Program structure
typedef struct {
    Instruction* instructions;
    int num_instructions;
    Variable* variables;
    int num_variables;
    int pid;
} Program;

// Interpreter context (for process execution)
typedef struct {
    PCB* process;
    Program* program;
    int current_instruction;  // Program Counter value
    int is_blocked;           // 1 if waiting on resource, 0 otherwise
    ResourceType waiting_resource;  // Which resource the process is waiting on
} InterpreterContext;

// ============= Function Declarations =============

/**
 * Load a program file and parse it into instructions
 * Filename: "program1.txt", "program2.txt", or "program3.txt"
 * Returns: Pointer to Program structure, NULL on error
 */
Program* loadProgram(const char* filename);

/**
 * Parse a single line of text into an Instruction
 * Returns: Instruction structure with parsed data
 */
Instruction parseInstruction(const char* line);

/**
 * Execute the next instruction for a given process
 * Returns: 0 if instruction executed successfully
 *          1 if process should be blocked (waiting for resource)
 *          2 if process finished
 *          -1 on error
 */
int executeInstruction(InterpreterContext* context);

/**
 * Get variable value by name for a process
 * Returns: pointer to variable, NULL if not found
 */
Variable* getVariable(Program* program, const char* var_name);

/**
 * Set variable value
 * Returns: 0 on success, -1 if variable not found
 */
int setVariable(Program* program, const char* var_name, const char* value);

/**
 * Create a new variable in the program
 * Returns: 0 on success, -1 if limit reached
 */
int createVariable(Program* program, const char* var_name, const char* value);

/**
 * Free program resources
 */
void freeProgram(Program* program);

/**
 * Utility: Check if string is a number
 */
int isNumber(const char* str);

/**
 * Utility: Get resource type from string name
 */
ResourceType getResourceTypeFromString(const char* resource_name);

#endif
