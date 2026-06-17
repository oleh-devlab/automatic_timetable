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

    src/cli/schedule_control.cpp
    src/core/time_blocks.cpp
    src/core/scheduling_algorithm.cpp
    src/core/task.cpp
    src/utils/time_utils.cpp
    src/cli/console_io.cpp
    src/cli/interactive_ui.cpp
    src/io/persistence.cpp

    src/cli/schedule_control.h
    src/core/time_blocks.h
    src/core/scheduling_algorithm.h
    src/core/task.h
    src/utils/time_utils.h
    src/cli/console_io.h
    src/cli/interactive_ui.h
    src/io/persistence.h
)