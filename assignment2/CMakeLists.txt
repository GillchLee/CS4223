cmake_minimum_required(VERSION 3.29)
project(CS4223AAAA)

set(CMAKE_CXX_STANDARD 14)

include_directories(assignment2/classes)

add_executable(CS4223AAAA
        classes/Bus.cpp
        classes/Bus.h
        classes/Cache.cpp
        classes/Cache.h
        classes/CacheLine.cpp
        classes/CacheLine.h
        classes/coherence.cpp
        classes/Constants.h
        classes/CPU.cpp
        classes/CPU.h
        classes/DRAM.cpp
        classes/DRAM.h
        classes/BusTransaction.h
        classes/BusTransaction.cpp
        classes/Bus.cpp
        classes/CPU.cpp
        classes/cacheWires.cpp)