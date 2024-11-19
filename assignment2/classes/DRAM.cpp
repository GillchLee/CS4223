#include "DRAM.h"

DRAM::DRAM() : DRAM(){}

void DRAM::execute(Bus *bus) {
    if (isBusy && cyclesLeft == 0) {
        isBusy = false;
        bus->putOnBus(BusTransaction::ReadResponseTransaction(address));
    } else if (isBusy) {
        cyclesLeft--;
    }
    if (!isBusy && !queue.empty()) {
        int newAddress = queue.front();
        queue.pop();
        isBusy = true;
        cyclesLeft = 99;
        address = newAddress;
    }
}

void DRAM::putOnDRAM(BusTransaction *bt) {
    if (bt->isValid){
    queue.push(bt->address);
    }
}
