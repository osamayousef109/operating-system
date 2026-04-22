cmake_minimum_required(VERSION 3.20)
project(OS_Simulation C)

set(CMAKE_C_STANDARD 11)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/memory)
include_directories(${CMAKE_SOURCE_DIR}/scheduler)
include_directories(${CMAKE_SOURCE_DIR}/execution)

# List your files here. As you create .c files, add them to this list.
add_executable(OS_Simulation
        main.c
        
        # Core Data Structures
        core/pcb.h
        core/pcb.c
        core/queues.h
        core/queues.c
        core/mutex.h
        core/mutex.c
        
        # Memory Management
        memory/memory.h
        memory/memory.c
        
        # Scheduler
        scheduler/scheduler.h
        scheduler/scheduler.c
)