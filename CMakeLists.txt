cmake_minimum_required(VERSION 3.29)
project(CS4223AAAA)

set(CMAKE_CXX_STANDARD 14)

include_directories(assignment2/classes)

add_executable(CS4223AAAA
    assignment2/classes/Bus.cpp
    assignment2/classes/Bus.h
    assignment2/classes/Cache.cpp
    assignment2/classes/Cache.h
    assignment2/classes/CacheLine.cpp
    assignment2/classes/CacheLine.h
    assignment2/classes/coherence.cpp
    assignment2/classes/Constants.h
    assignment2/classes/CPU.cpp
    assignment2/classes/CPU.h
    assignment2/classes/DRAM.cpp
    assignment2/classes/DRAM.h
        assignment2/classes/BusTransaction.h
        assignment2/classes/BusTransaction.cpp)
