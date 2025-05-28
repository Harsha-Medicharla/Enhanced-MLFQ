cmake_minimum_required(VERSION 3.10)
project(MLFQ_Scheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(mlfq MLFQ_2PROCESSOR.cpp)

# Set compiler flags
target_compile_options(mlfq PRIVATE -Wall -Wextra -O2)