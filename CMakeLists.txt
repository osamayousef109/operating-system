cmake_minimum_required(VERSION 3.20)
project(OS_Simulation C)

set(CMAKE_C_STANDARD 11)

# List your files here. As you create .c files, add them to this list.
add_executable(OS_Simulation
        main.c
        pcb.h
        memory.h
        memory.c
        pcb.c
        main.c
)