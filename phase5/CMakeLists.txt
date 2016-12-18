cmake_minimum_required(VERSION 3.2)
project(phase5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(SOURCE_FILES
    main.cpp
    VirtualMachine.cpp
    VirtualMachine.h
    VirtualMachineMemory.cpp
    VirtualMachineMemory.h)

add_executable(vm ${SOURCE_FILES})