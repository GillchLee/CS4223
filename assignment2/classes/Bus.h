
#ifndef BUS_H
#define BUS_H
#include <queue>

#include "BusTransaction.h"
#include "DRAM.h"

class Bus {
std::queue<BusTransaction*> queue;
public:
    int busTraffic = 0;
    BusTransaction *currentTransaction = nullptr;

    void execute(DRAM *dram);
    void putOnBus(BusTransaction *b);
};

#endif //BUS_H
