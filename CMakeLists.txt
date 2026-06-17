cmake_minimum_required(VERSION 3.10)

project(AutomaticTimeTable)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "Choose the type of build" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-ffunction-sections -fdata-sections)
    add_link_options(-Wl,--gc-sections)
    add_link_options(-s)
endif()

add_executable(app
    src/main.cpp

    src/schedule_control.cpp
    src/time_blocks.cpp
    src/scheduling_algorithm.cpp
    src/task.cpp
    src/time_utils.cpp
    src/console_io.cpp
    src/interactive_ui.cpp
    src/persistence.cpp

    src/schedule_control.h
    src/time_blocks.h
    src/scheduling_algorithm.h
    src/task.h
    src/time_utils.h
    src/console_io.h
    src/interactive_ui.h
    src/persistence.h
)