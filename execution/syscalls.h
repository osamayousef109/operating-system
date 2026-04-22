#ifndef SYSCALLS_H
#define SYSCALLS_H

#define MAX_FILENAME 100
#define MAX_FILE_CONTENT 1000
#define DISK_FILES_DIR "disk_files"

/**
 * Print text to screen
 * Protected by userOutput mutex
 */
void syscall_print(const char* value);

/**
 * Get text input from user
 * Protected by userInput mutex
 */
void syscall_input(char* buffer);

/**
 * Write data to a file
 * Protected by file mutex
 * Creates file if it doesn't exist
 */
void syscall_writeFile(const char* filename, const char* data);

/**
 * Read data from a file
 * Protected by file mutex
 * Returns: Allocated string with file content, NULL if file not found
 * Caller must free returned string
 */
char* syscall_readFile(const char* filename);

/**
 * Initialize syscalls (create disk_files directory if needed)
 */
void syscalls_init();

#endif
