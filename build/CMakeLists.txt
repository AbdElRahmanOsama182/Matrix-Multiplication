cmake_minimum_required(VERSION 3.10)
project(MatrixMul)

# Set C standard to C11
set(CMAKE_C_STANDARD 11)

# Add your source file
set(SOURCE_FILES ../MatrixMul.c)

# Add an executable target
add_executable(MatrixMul ${SOURCE_FILES})

# Find pthread library (for POSIX threads)
find_package(Threads REQUIRED)

# Link pthread library
target_link_libraries(MatrixMul Threads::Threads)
