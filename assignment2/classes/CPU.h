#ifndef CPU_H
#define CPU_H

#include <iomanip>
#include <iostream>

#include "Bus.h"
#include "Cache.h"
#include "DRAM.h"

class CPU {
private:
    int blockSize;      // Size of each cache block in bytes
    int id;           // CPU ID
    Cache* cache;      // Private cache
    Bus* bus;          // Shared bus
    DRAM* dram;        // Shared DRAM
    int cycles;        // Cycle counter
    bool on_process;    // if CPU is working or not
    Constants::MESI_States state = Constants::NO_State;
    bool readRequestSent = false;


    int label;
    unsigned int data;
    int target_cycles;



    // benchmark factors
    unsigned int total_instructions; // for computing IPC
    unsigned long total_cycles;  // idle cycles + target cycles
    unsigned int num_ls;        // # of load/store
    unsigned long compute_cycles; // # of total compute cycles
    unsigned long cache_hit;
    unsigned long cache_miss;
    unsigned long idleCycles= 0;
    unsigned long dataTraffic=0;
    unsigned long sharedData=0;
    unsigned long nonSharedData=0;
public:
    CPU(int blockSize,int id, Cache* cache, Bus* bus, DRAM* dram, int cycles, bool on_process, int label, unsigned int data,
    int target_cycles,
    unsigned int total_instructions, long total_cycles, unsigned int num_ls,unsigned long compute_cycles, unsigned long cache_hit, unsigned long cache_miss)
        : blockSize(blockSize), id(id), cache(cache), bus(bus), dram(dram), cycles(0), on_process(false),label(-1), data(0),
        target_cycles(100),
        total_instructions(0), total_cycles(0), num_ls(0), compute_cycles(0), cache_hit(0),cache_miss(0) {}

    // Function to read operations from a file
    bool Execute(int input_label, int input_data);

    void resetState();

    void snoop();

    void PrintStats() const {
        double IPC = static_cast<double>(nonSharedData) / static_cast<double>(sharedData + nonSharedData);
        std::cout << "Total cycles:" << total_cycles << std::endl;
        std::cout << "Total instructions:" << total_instructions << std::endl;
        std::cout << "Number of Load/Store Operations: " << num_ls << std::endl;
        std::cout << "Compute Cycles: " << compute_cycles << std::endl;
        std::cout << "Cache hit : " << cache_hit << std::endl;
        std::cout << "Cache miss : " << cache_miss << std::endl;
        std::cout << "Idle cycles : " << idleCycles << std::endl;
        std::cout << "Distribution of accesses to private data : " << IPC << std::endl;
        std::cout << "Shared data : " << sharedData << std::endl;
        std::cout << "exclusive data : " << nonSharedData << std::endl;
        std::cout <<"\n-----------------------------------------------------------------------------------" << std::endl;

    }
};

#endif //CPU_H
