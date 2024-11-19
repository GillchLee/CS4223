#ifndef DRAM_H
#define DRAM_H
#include <queue>

#include "Bus.h"

class Bus;

class DRAM {
    bool isBusy = false;
    int cyclesLeft = 0;
    int address = 0;
    std::queue<int> queue;

public:

    void execute(Bus *bus);
    void putOnDRAM(BusTransaction *bt);

    DRAM(){}
};

#endif //DRAM_H
