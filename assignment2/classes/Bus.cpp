#include "Bus.h"

void Bus::execute(DRAM *dram) {
    if (currentTransaction != nullptr) {
        busTraffic++;
        if (currentTransaction->size == 0) {
            if (currentTransaction->type == BusTransaction::ReadShared || currentTransaction->type == BusTransaction::ReadExclusive) {
                dram->putOnDRAM(currentTransaction);
            }
            // delete currentTransaction;
            if (!queue.empty()) {
                currentTransaction = queue.front();
                queue.pop();
            }
        }else {
            currentTransaction->size--;
        }
    } else if (!queue.empty()) {
        currentTransaction = queue.front();
        queue.pop();
    }
}

void Bus::putOnBus(BusTransaction *b) {
    queue.push(b);
}

