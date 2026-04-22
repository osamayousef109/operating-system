# Operating System Simulator - Project Structure

## Directory Organization

```
operating-system/
│
├── core/                          # Core Data Structures
│   ├── pcb.h / pcb.c             # Process Control Block - process metadata
│   ├── queues.h / queues.c       # Queue implementation - Ready and Blocked queues
│   └── mutex.h / mutex.c         # Mutex/Semaphore system - mutual exclusion
│
├── memory/                        # Memory Management
│   ├── memory.h / memory.c       # Main memory (40 words) and management
│   └── disk.c (to create)        # Disk swapping functionality
│
├── scheduler/                     # Scheduling & Process Management
│   ├── scheduler.h / scheduler.c # RR, HRRN, MLFQ algorithms
│   └── timing.c (optional)       # Time management
│
├── execution/ (to create)        # Program Execution & Interpretation
│   ├── interpreter.h/c           # Parser and instruction executor
│   ├── syscalls.h/c              # System calls implementation
│   └── process.h/c               # Process execution engine
│
├── main.c                         # Entry point - initializes OS
├── CMakeLists.txt                # Build configuration
└── Documentation/
    ├── PROJECT_STRUCTURE.md      # This file
    ├── MUTEX_IMPLEMENTATION.md
    ├── MUTEX_USAGE_EXAMPLE.c
    └── README.md
```

## Module Descriptions

### core/ - Core Data Structures

**Purpose**: Fundamental data structures used by all OS components

**Files**:

- **pcb.h/c**: Process Control Block
  - Struct containing process metadata: pid, state, PC, memory boundaries, timing info
  - Used by: scheduler, memory manager, queues
- **queues.h/c**: Queue data structure
  - Linked-list based queue implementation
  - Used for: Ready Queue, Blocked Queue, Mutex blocked queues
  - Functions: push, pop, empty, removeProcessByPID
  - Used by: scheduler, mutex, all modules needing process collections
- **mutex.h/c**: Mutual Exclusion (Semaphore) System
  - 3 global mutexes: file, userInput, userOutput
  - Operations: semWait (acquire), semSignal (release)
  - Handles: process blocking, FIFO wakeup ordering
  - Used by: system calls, program executor

### memory/ - Memory Management

**Purpose**: Manage the 40-word main memory and disk swapping

**Files**:

- **memory.h/c**: Main memory management
  - 40-word shared memory for all processes
  - Memory words store variable name + value pairs
  - Functions: initializeMemory, printMemory, allocate, deallocate
  - To add: Memory fragmentation handling, boundary checking
- **disk.c** (to create): Disk swapping
  - Save/load process memory to disk when needed
  - Handles: memory overflow, process restoration
  - Used by: memory manager when memory is full

### scheduler/ - Scheduling & Process Management

**Purpose**: Schedule processes and manage timing

**Files**:

- **scheduler.h/c**: Scheduling algorithms
  - Implements: RR (Round Robin), HRRN (Highest Response Ratio Next), MLFQ (Multi-Level Feedback Queue)
  - Selects which process runs next
  - Handles: context switching, preemption, process state transitions
  - Main function: runOS(Algorithm)

### execution/ (to create) - Program Execution

**Purpose**: Execute actual programs and manage system calls

**Files**:

- **interpreter.h/c**: Program parser and executor
  - Reads program text files
  - Parses commands: print, assign, writeFile, readFile, printFromTo, semWait, semSignal
  - Manages: instruction fetching, Program Counter
- **syscalls.h/c**: System calls
  - Implements: read file, write file, input, output, memory read/write
  - Bridges: program instructions to OS resources
  - Handles: resource permissions, error checking
- **process.h/c**: Process execution engine
  - Executes individual instructions
  - Updates Program Counter, process state
  - Handles: instruction-level blocking, errors

### root/ - Initialization & Configuration

**Files**:

- **main.c**: Entry point
  - Initializes all subsystems: memory, mutexes, scheduler
  - Starts OS simulation
  - Calls: initializeMemory(), initializeMutexes(), runOS()

- **CMakeLists.txt**: Build configuration
  - Specifies include directories for each module
  - Lists all source files to compile
  - Compiler settings: C11 standard

## Data Flow Between Modules

```
main.c
  ├─→ Initialize: memory/ scheduler/ core/
  └─→ runOS(algorithm)
         │
         ├─→ scheduler/scheduler.c
         │   ├─→ Select process from core/queues.c
         │   ├─→ Manage core/pcb.c processes
         │   └─→ Dispatch to execution module
         │
         ├─→ execution/interpreter.c (ADD)
         │   ├─→ Parse program instruction
         │   ├─→ Access memory/memory.c for variables
         │   ├─→ Check core/mutex.c for resources
         │   └─→ Call execution/syscalls.c
         │
         ├─→ execution/syscalls.c (ADD)
         │   ├─→ File operations → memory/disk.c
         │   ├─→ Resource access → core/mutex.c
         │   └─→ Memory access → memory/memory.c
         │
         └─→ core/mutex.c
             ├─→ Block process → core/queues.c
             └─→ Wake process → core/queues.c
```

## Compilation

With proper CMakeLists.txt configuration, compilation is:

```bash
cd cmake-build-debug
cmake ..
cmake --build .
```

The build system automatically:

- Includes directories from core/, memory/, scheduler/, execution/
- Compiles all .c files
- Links all object files
- Produces: OS_Simulation executable

## Including Files

Since include directories are set up in CMakeLists.txt, use:

```c
#include "pcb.h"          // From core/
#include "memory.h"       // From memory/
#include "scheduler.h"    // From scheduler/
#include "mutex.h"        // From core/
#include "queues.h"       // From core/
```

No need for relative paths like `#include "../core/pcb.h"`

## Adding New Modules

To add a new module (e.g., execution/interpreter.h):

1. Create directory: `execution/`
2. Place implementation: `execution/interpreter.c` and `execution/interpreter.h`
3. Update CMakeLists.txt:
   ```cmake
   include_directories(${CMAKE_SOURCE_DIR}/execution)
   # Add to add_executable: execution/interpreter.h execution/interpreter.c
   ```
4. Include in other files: `#include "interpreter.h"`

## Next Steps

1. **Execution Module**: Create `execution/interpreter.c` to parse and execute programs
2. **System Calls Module**: Create `execution/syscalls.c` for file I/O and resources
3. **Disk Module**: Create `memory/disk.c` for memory swapping
4. **Integration**: Connect all modules together in scheduler

---

**Status**: Project restructured and ready for continued development ✓
**Compilation**: All files compile successfully ✓
**Next Priority**: Implement execution/interpreter module
