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
        BusTransaction newTransaction = queue.front();
        queue.pop();
        while (!queue.empty() && !newTransaction.isValid) {
            newTransaction = queue.front();
            queue.pop();
        }
        if (newTransaction.isValid) {
            isBusy = true;
            cyclesLeft = 99;
            address = newTransaction.address;
        }
    }
}