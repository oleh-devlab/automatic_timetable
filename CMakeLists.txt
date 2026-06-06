cmake_minimum_required(VERSION 3.10)
project(AutomaticTimeTable)
add_executable(app
    src/main.cpp

    src/schedule.cpp
    src/structs.cpp
    src/scheduling_algorithm.cpp

    src/schedule.h
    src/structs.h
    src/scheduling_algorithm.h
)