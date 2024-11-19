
#ifndef BUS_H
#define BUS_H
#include "BusTransaction.h"


//TODO: Implement bus state that it broadcasts to the entire system. Also implement a queue for who drives the bus.
extern unsigned int busTraffic;

class Bus {
private:
    bool isOccupied = false;

public:
    void putOnBus(BusTransaction transaction);
};

#endif //BUS_H
