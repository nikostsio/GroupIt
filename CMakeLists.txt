cmake_minimum_required(VERSION 3.10)
project(GroupItSolver)

set(CMAKE_CXX_STANDARD 17)

# Tell CMake where to find the header files
include_directories(include)

# Define the source files
set(SOURCES
    src/main.cpp
    src/ImageUtils.cpp
    src/GameSolver.cpp
    src/DeviceController.cpp
)

# Create the executable
add_executable(GroupItSolver ${SOURCES})