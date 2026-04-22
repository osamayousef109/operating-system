#ifndef INTERPRETER_H
#define INTERPRETER_H

// Reads a program text file line by line.
void loadProgram(char* filename);
void parseAndExecute(char* instruction);

#endif
