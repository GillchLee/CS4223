#ifndef DRAM_H
#define DRAM_H
#include <queue>

#include "Bus.h"


class DRAM {
    bool isBusy = false;
    int cyclesLeft = 0;
    int address = 0;
    std::queue<BusTransaction> queue;

public:

    void execute(Bus *bus);
    void putOnDRAM(int address);

    DRAM();
};

#endif //DRAM_H
