#include "Bus.h"

void Bus::execute(DRAM *dram) {
    if (currentTransaction != nullptr) {
        busTraffic++;
        if (currentTransaction->size == 0) {
            if (currentTransaction->type == BusTransaction::ReadShared || currentTransaction->type == BusTransaction::ReadExclusive) {
                dram->putOnDRAM(currentTransaction);
            }
            delete currentTransaction;
            currentTransaction = queue.front();
            queue.pop();
        }else {
            currentTransaction->size--;
        }
    }
}

void Bus::putOnBus(BusTransaction *b) {
    queue.push(b);
}

