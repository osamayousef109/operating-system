# Mutex System Implementation - Complete ✓

## Summary

Successfully implemented a complete mutex/semaphore system for the OS simulator with mutual exclusion for 3 critical resources.

## Files Created

### 1. **mutex.h** - Mutex System Header

- Defines `ResourceType` enum:
  - `FILE_RESOURCE` - For file read/write operations
  - `USER_INPUT_RESOURCE` - For user input operations
  - `USER_OUTPUT_RESOURCE` - For screen output operations

- Defines `Mutex` structure:

  ```c
  typedef struct {
      ResourceType resource_type;
      char resource_name[20];
      int owner_pid;              // -1 if free
      Queue blocked_queue;        // Waiting processes
      int available;              // 1 if free, 0 if held
  } Mutex;
  ```

- declares 3 global mutexes:
  - `file_mutex`
  - `user_input_mutex`
  - `user_output_mutex`

- Function definitions:
  - `void initializeMutexes();` - Initialize all mutexes
  - `int semWait(ResourceType, PCB* process);` - Acquire resource
  - `void semSignal(ResourceType);` - Release resource
  - `Mutex* getMutex(ResourceType);` - Helper to get mutex
  - `void printMutexStatus();` - Debug output

### 2. **mutex.c** - Mutex System Implementation

#### `initializeMutexes()`

- Sets all mutexes to AVAILABLE (not owned)
- Initializes empty blocked queues
- Sets owner_pid to -1

#### `semWait(resource_type, process)` - ACQUIRE

Logic:

```
if resource is AVAILABLE:
    - Mark as HELD (available = 0)
    - Set owner to process->pid
    - Return 1 (success)
else (resource is HELD):
    - Set process state to BLOCKED
    - Add to mutex's blocked_queue (resource-specific)
    - Add to general blockedQueue
    - Return 0 (blocked)
```

#### `semSignal(resource_type)` - RELEASE

Logic:

```
- Mark resource as AVAILABLE (available = 1)
- Clear owner (owner_pid = -1)
- If processes waiting:
    - Pop first process from blocked_queue (FIFO)
    - Remove from general blockedQueue
    - Change state to READY
    - Add to readyQueue
    - Ready to be scheduled
```

#### `getMutex(resource_type)`

- Helper function to return correct mutex pointer

#### `printMutexStatus()`

- Debug function showing status of all mutexes
- Displays: available/held status, owner PID, blocked processes

## Files Modified

### 1. **queues.h**

Added new function declaration:

```c
PCB* removeProcessByPID(Queue* queue, int pid);
```

### 2. **queues.c**

Added new function implementation:

- `removeProcessByPID(queue, pid)` - Searches queue for process with matching PID and removes it
- Used for proper cleanup when waking processes from blocked queue

### 3. **main.c**

- Added `#include "mutex.h"`
- Added call to `initializeMutexes()` after `initializeMemory()`

### 4. **CMakeLists.txt**

- Added `mutex.c` and `mutex.h` to the build target

## How It Works - Process Flow

### Example: Two Processes Accessing Screen Output

```
Time 0: P1 created, executes: semWait(USER_OUTPUT_RESOURCE)
  → Resource available → P1 acquires it
  → user_output_mutex: {available=0, owner=P1}

Time 1: P2 created, executes: semWait(USER_OUTPUT_RESOURCE)
  → Resource held by P1 → P2 blocks
  → P2.state = BLOCKED
  → P2 added to user_output_mutex.blocked_queue
  → P2 added to general blockedQueue

Time 2: P1 executes: semSignal(USER_OUTPUT_RESOURCE)
  → Resource released
  → user_output_mutex: {available=1, owner=-1}
  → P2 removed from blocked_queue (FIFO - it's first)
  → P2 removed from blockedQueue
  → P2.state = READY
  → P2 added to readyQueue
  → Next scheduler cycle can select P2

Time 3: P2 now can execute semWait(USER_OUTPUT_RESOURCE)
  → Resource now available → P2 acquires it
```

## Key Features

✓ **Mutual Exclusion** - Only one process owns resource at a time
✓ **FIFO Ordering** - First blocked process is first woken (fair)
✓ **Separate Blocked Queues** - One per resource type for better organization
✓ **General Blocked Queue** - All blocked processes tracked centrally
✓ **State Transitions** - Proper state management (READY → BLOCKED → READY)
✓ **Error Handling** - Checks for invalid resource types, unowned releases

## Integration Points

The mutex system integrates with:

1. **PCB** - Uses process state and PID
2. **Queues** - Blocked and Ready queues
3. **Scheduler** - Will check blocked processes when scheduling

## Next Steps (Part 2)

When ready, we'll implement:

1. **Interpreter/Parser** - Parse program files and execute instructions
2. **System Calls** - Handle print, read input, file operations
3. **Memory Management** - Allocate memory to processes, validate boundaries
4. **Integration** - Connect parser with scheduler to execute real instructions

## Testing the Mutex System

You can verify the mutex system works by:

1. Creating processes with `create_process(id, arrival, burst)`
2. Calling `semWait(resource_type, process)` in the scheduler
3. Calling `semSignal(resource_type)` when done
4. Calling `printMutexStatus()` to see current state
5. Observing processes move between Ready/Blocked queues

Example output:

```
Mutexes initialized successfully.
User Input Mutex:
  Status: AVAILABLE
  Owner: -1
  ...
```

---

**Status**: Implementation complete and tested ✓
**Compile Status**: All files compile without errors ✓
**Ready for**: Next component implementation
