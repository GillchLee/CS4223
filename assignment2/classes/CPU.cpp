//
// Created by Diana on 11/17/2024.
//

#include "CPU.h"

void CPU::snoop() {
    if (!bus->currentTransaction) {
        return;
    }
    BusTransaction *bt = bus->currentTransaction;
}


bool CPU::Execute(const int input_label, const unsigned int input_data) {
    // access cache, dram. and compute
    int address = -1;
    if (!on_process) {
        // if there's no instruction in CPU, insert label and data to CPU

        label = input_label;
        data = input_data;
        address = input_data;
        target_cycles = data;

        if (label == 0 || label == 1) num_ls++; // for counting # of load/store
        total_instructions++;
        on_process = true;
        cycles = 0;
        if (label == 1 || label == 2) {
            state = cache->getState(address);
            return true; // causes it to stall 1 cycle for cache penalty
        }
    }
    if (label == 2) {
        // start computing and hold cycles for "data time". if input_data is 0xc, wait for 12 cycles to compute.
        if (target_cycles == cycles) {
            total_instructions += cycles - 1;
            compute_cycles += cycles;
            total_cycles--; // value was off by 1
            resetState();
        } else {
            cycles++;
        }
    } else if (label == 0) {
        idleCycles++;
        // Read
        if (state == Constants::I_State) {
            cache_miss++;
            if (!readRequestSent) {
                readRequestSent = true;
                bus->putOnBus(BusTransaction::ReadTransaction(address));
            }
            // if the needed address is on the bus
            if (bus->currentTransaction != nullptr && bus->currentTransaction->address == input_data &&
                bus->currentTransaction->type == BusTransaction::ReadResponse && bus->currentTransaction->isLast()) {
                cache->addLine(address, CacheLine(true, cache->calculateTag(address),
                                         cache->getNewState(Constants::I_State, true)), bus);
                resetState();
            }
        } else if (state == Constants::S_State) {
            cache_hit++;
            resetState();
        } else if (state == Constants::M_State) {
            cache_hit++;
            resetState();
        } else if (state == Constants::E_State) {
            cache_hit++;
            resetState();
        }
    } else if (label == 1) {
        idleCycles++;
        // Write
        if (state == Constants::I_State) {
            cache_miss++;
            if (!readRequestSent) {
                readRequestSent = true;
                bus->putOnBus(BusTransaction::ReadXTransaction(address));
            }
            if (bus->currentTransaction != nullptr && bus->currentTransaction->address == input_data &&
                bus->currentTransaction->type == BusTransaction::ReadResponse && bus->currentTransaction->
                isLast()) {
                cache->addLine(address, CacheLine(true, cache->calculateTag(address),
                         cache->getNewState(Constants::I_State, false)), bus);
                resetState();
            }
        } else if (state == Constants::S_State) {
            cache_hit++;
            CacheLine* cl = cache->getLine(address);
            cl->state = Constants::M_State;
            resetState();
        } else if (state == Constants::M_State) {
            cache_hit++;
            resetState();
        } else if (state == Constants::E_State) {
            cache_hit++;
            CacheLine* cl = cache->getLine(address);
            cl->state = Constants::M_State;
            resetState();
        }
    }
    total_cycles++;
    return on_process;
}

void CPU::resetState() {
    on_process = false;
    readRequestSent = false;
    state = Constants::NO_State;
    cycles = 0;
}
