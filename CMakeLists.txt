cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(AutomaticTimeTable)
add_executable(app
    src/main.cpp

    src/schedule_control.cpp
    src/time_blocks.cpp
    src/scheduling_algorithm.cpp
    src/task.cpp

    src/schedule_control.h
    src/time_blocks.h
    src/scheduling_algorithm.h
    src/task.h
)