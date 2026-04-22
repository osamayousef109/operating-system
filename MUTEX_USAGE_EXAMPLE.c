/**
 * MUTEX SYSTEM USAGE EXAMPLE
 * 
 * This file demonstrates how the mutex system works
 * 
 * Three Mutexes available:
 * 1. FILE_RESOURCE - for file read/write operations
 * 2. USER_INPUT_RESOURCE - for user input operations  
 * 3. USER_OUTPUT_RESOURCE - for screen output operations
 */

#include "mutex.h"
#include "pcb.h"

// Example of how mutex system works:

/*
INITIALIZATION:
- initializeMutexes() is called in main()
- All mutexes start as AVAILABLE (not owned by any process)

USAGE PATTERN:

1. ACQUIRE A RESOURCE (semWait):
   -----------------------------------
   PCB* process = create_process(1, 0, 10);   // Process 1
   
   int result = semWait(USER_OUTPUT_RESOURCE, process);
   
   if (result == 1) {
       // SUCCESS - Got the resource
       printf("Process 1: Printing to screen...\n");
   } else {
       // FAILED - Process was blocked
       // Process state is set to BLOCKED
       // Process added to:
       //   - user_output_mutex.blocked_queue (resource-specific)
       //   - blockedQueue (general blocked queue)
   }

2. RELEASE A RESOURCE (semSignal):
   --------------------------------
   semSignal(USER_OUTPUT_RESOURCE);
   
   What happens:
   - Resource becomes AVAILABLE
   - Owner is cleared
   - If processes are waiting on this resource:
     * First waiting process is moved to READY queue
     * It's removed from both blocked queues
     * On next scheduler cycle, it can be selected

MUTEX STATES:
- AVAILABLE (1): Free for any process to acquire
- HELD (0): Owned by a process with PID stored

PROCESS FLOW EXAMPLE:
--------------------
Time 0:
- Process 1 created (READY)
- Process 1 executes: semWait(USER_OUTPUT_RESOURCE)
- Result: ACQUIRED (available=1 → available=0, owner=P1)

Time 1:
- Process 2 created (READY)
- Process 2 tries: semWait(USER_OUTPUT_RESOURCE)
- Result: BLOCKED (available=0, so P2 blocked)
  - P2 state → BLOCKED
  - P2 added to user_output_mutex.blocked_queue
  - P2 added to general blockedQueue

Time 2:
- Process 1 executes: semSignal(USER_OUTPUT_RESOURCE)
- Result: RELEASED (owner=-1, available=1)
  - P2 is in blocked queue waiting
  - P2 is woken up and moved to READY queue
  - P2 can now be scheduled

BLOCKING SCENARIO (Multiple Processes):
----------------------------------------
If Process 3 also waits while P2 is blocked:
1. P2 blocks (moves to blocked queue)
2. P3 tries semWait, also blocks (added to queue after P2)
3. When semSignal is called:
   - P2 is woken (FIFO - first in, first out)
   - P3 remains blocked until P2 releases

PRINTING MUTEX STATUS:
---------------------
printMutexStatus();  // Displays all mutex information
*/

// The mutex system ensures:
// ✓ Mutual Exclusion - Only one process uses a resource at a time
// ✓ FIFO Waiting - First blocked process is first woken up
// ✓ No Deadlock - Processes can't hold multiple different resources simultaneously
//                 (based on project design - they acquire, use, release one at a time)
