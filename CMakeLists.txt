cmake_minimum_required(VERSION 3.20)
project(OS_Simulation C)

set(CMAKE_C_STANDARD 11)
# --- Fetch Raylib Automatically ---
include(FetchContent)
FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/master.tar.gz
)
FetchContent_MakeAvailable(raylib)

add_executable(OS_Simulation
        main.c

        # Core Data Structures
        core/pcb.h
        core/pcb.c
        core/queues.h
        core/queues.c

        # Memory Management
        memory/memory.h
        memory/memory.c

        # Scheduler
        scheduler/scheduler.h
        scheduler/scheduler.c
        core/mutex.h
        core/mutex.c
        execution/syscalls.h
        execution/syscalls.c
        execution/interpreter.c
        execution/interpreter.h
)
# --- Link Raylib to your project ---
target_link_libraries(OS_Simulation raylib)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/memory)
#include_directories(${CMAKE_SOURCE_DIR}/scheduler)
#include_directories(${CMAKE_SOURCE_DIR}/execution)

# List your files here. As you create .c files, add them to this list.
