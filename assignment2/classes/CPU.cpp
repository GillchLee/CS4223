//
// Created by Diana on 11/17/2024.
//

#include "CPU.h"

#include <assert.h>

void CPU::snoop() {
    if (!bus->currentTransaction || (bus->currentTransaction->type != BusTransaction::ReadExclusive && bus->
                                     currentTransaction->type != BusTransaction::ReadShared && bus->currentTransaction->
                                     type != BusTransaction::Invalidate)) {
        return;
    }
    BusTransaction *bt = bus->currentTransaction;
    if (!cache->cacheContains(bt->address)) {
        return;
    }
    CacheLine *cl = cache->getLine(bt->address);
    Constants::MESI_States state = cl->state;
    if (state == Constants::I_State) {
        return;
    }
    if (bt->type == BusTransaction::Invalidate) {
        // bus->invalidationsCnt++;
        cache->removeLine(bt->address);
    }
    if (bt->type == BusTransaction::ReadExclusive) {
        // bus->invalidationsCnt++;
        if (state == Constants::E_State) {
            bt->isValid = false;
            bus->putOnBus(BusTransaction::ReadResponseTransaction(bt->address));
        } else if (state == Constants::M_State) {
            bt->isValid = false;
            bus->putOnBus(BusTransaction::ReadResponseTransaction(bt->address));
            bus->putOnBus(BusTransaction::WriteBackTransaction());
        }
        cache->removeLine(bt->address);
    } else if (bt->type == BusTransaction::ReadShared) {
        if (state == Constants::E_State) {
            bt->isValid = false;
            bus->putOnBus(BusTransaction::ReadResponseTransaction(bt->address));
            cl->state = Constants::S_State;
        } else if (state == Constants::M_State) {
            bt->isValid = false;
            bus->putOnBus(BusTransaction::ReadResponseTransaction(bt->address));
            bus->putOnBus(BusTransaction::WriteBackTransaction());
            cl->state = Constants::S_State;
        }
    }
}


bool CPU::Execute(const int input_label, const int input_data) {
    total_cycles++;
    // access cache, dram. and compute
    if (!on_process) {
        // if there's no instruction in CPU, insert label and data to CPU

        label = input_label;
        data = input_data;
        target_cycles = data;

        on_process = true;
        cycles = 1;
        if (label == 0 || label == 1) {
            total_instructions++;
            num_ls++;
            state = cache->getState(input_data);
            return true; // causes it to stall 1 cycle for cache penalty
        }
    }
    if (label == 2) {
        // start computing and hold cycles for "data time". if input_data is 0xc, wait for 12 cycles to compute.
        if (target_cycles == cycles) {
            total_instructions += cycles;
            compute_cycles += cycles;
            resetState();
        } else {
            cycles++;
        }
    } else if (label == 0) {
        idleCycles++;
        // Read
        if (state == Constants::I_State) {
            if (!readRequestSent) {
                cache_miss++;
                readRequestSent = true;
                assert(input_data != 0);
                bus->putOnBus(BusTransaction::ReadTransaction(input_data));
            }
            // if the needed address is on the bus
            if (bus->currentTransaction != nullptr && bus->currentTransaction->address == input_data &&
                bus->currentTransaction->type == BusTransaction::ReadResponse && bus->currentTransaction->isLast()) {
                cache->addLine(input_data, CacheLine(true, cache->calculateTag(input_data),
                                                     cache->getNewState(Constants::I_State, true, input_data, bus)),
                               bus);
                resetState();
            }
        } else if (state == Constants::S_State) {
            cache_hit++;
            sharedData++;
            resetState();
        } else if (state == Constants::M_State) {
            cache_hit++;
            nonSharedData++;
            resetState();
        } else if (state == Constants::E_State) {
            cache_hit++;
            nonSharedData++;
            resetState();
        }
    } else if (label == 1) {
        idleCycles++;
        // Write
        if (state == Constants::I_State) {
            if (!readRequestSent) {
                cache_miss++;
                readRequestSent = true;
                bus->putOnBus(BusTransaction::ReadXTransaction(input_data));
            }
            if (bus->currentTransaction != nullptr && bus->currentTransaction->address == input_data &&
                bus->currentTransaction->type == BusTransaction::ReadResponse && bus->currentTransaction->
                isLast()) {
                cache->addLine(input_data, CacheLine(true, cache->calculateTag(input_data),
                                                     cache->getNewState(Constants::I_State, false, input_data, bus)),
                               bus);
                resetState();
            }
        } else if (state == Constants::S_State) {
            cache_hit++;
            sharedData++;
            // TODO: Check if this does anything
            CacheLine *cl = cache->getLine(input_data);
            cl->state = Constants::M_State;
            bus->putOnBus(BusTransaction::InvalidateTransaction(input_data));
            resetState();
        } else if (state == Constants::M_State) {
            cache_hit++;
            nonSharedData++;
            resetState();
        } else if (state == Constants::E_State) {
            cache_hit++;
            nonSharedData++;
            if (cache->getState(input_data) != Constants::I_State) {
                CacheLine *cl = cache->getLine(input_data);
                cl->state = Constants::M_State;
            }
            resetState();
        }
    }
    return on_process;
}

void CPU::resetState() {
    on_process = false;
    readRequestSent = false;
    state = Constants::NO_State;
    cycles = 0;
}

bool CPU::ExecuteDragon(int input_label, int input_data) {
    total_cycles++;
    // access cache, dram. and compute
    if (!on_process) {
        // if there's no instruction in CPU, insert label and data to CPU

        label = input_label;
        data = input_data;
        target_cycles = data;

        if (label == 0 || label == 1) num_ls++; // for counting # of load/store
        total_instructions++;
        on_process = true;
        cycles = 0;
        if (label == 0 || label == 1) {
            //TODO:fix this/update getState or create new getState, remember this updates the order of the cache(LRU)
            state = cache->getState(input_data);
            return true; // causes it to stall 1 cycle for cache penalty
        }
    }
    if (label == 2) {
        // start computing and hold cycles for "data time". if input_data is 0xc, wait for 12 cycles to compute.
        if (target_cycles == cycles) {
            total_instructions += cycles - 1;
            compute_cycles += cycles;
            resetState();
        } else {
            cycles++;
        }
    } else if (label == 0) {
        idleCycles++;
        // Read
        if (state == Constants::I_State) {
            if (!readRequestSent) {
                cache_miss++;
                readRequestSent = true;
                bus->putOnBus(BusTransaction::ReadTransaction(input_data));
            }
            if (bus->currentTransaction != nullptr && bus->currentTransaction->address == input_data &&
                bus->currentTransaction->type == BusTransaction::ReadResponse && bus->currentTransaction->
                isLast()) {
                cache->addLine(input_data, CacheLine(true, cache->calculateTag(input_data),
                                                     cache->getNewDragonState(Constants::I_State, true, input_data)),
                               bus);
                resetState();
            }
        } else if (state == Constants::E_State) {
            cache_hit++;
            nonSharedData++;
            resetState();
        } else if (state == Constants::M_State) {
            cache_hit++;
            nonSharedData++;
            resetState();
        } else if (state == Constants::Sm_State) {
            cache_hit++;
            sharedData++;
            resetState();
        } else if (state == Constants::Sc_State) {
            cache_hit++;
            sharedData++;
            resetState();
        }
    } else if (label == 1) {
        idleCycles++;
        // Write
        if (state == Constants::I_State) {
            if (!readRequestSent) {
                cache_miss++;
                readRequestSent = true;
                bus->putOnBus(BusTransaction::ReadTransaction(input_data));
            }
            if (bus->currentTransaction != nullptr && bus->currentTransaction->address == input_data &&
                bus->currentTransaction->type == BusTransaction::ReadResponse && bus->currentTransaction->
                isLast()) {
                cache->addLine(input_data, CacheLine(true, cache->calculateTag(input_data),
                                                     cache->getNewDragonState(Constants::I_State, false, input_data)),
                               bus);
                if (cache->getNewDragonState(Constants::I_State, false, input_data) == Constants::Sm_State) {
                    bus->putOnBus(BusTransaction::BusUpdateTransaction(input_data));
                }
                resetState();
            }
        } else if (state == Constants::E_State) {
            cache_hit++;
            nonSharedData++;
            CacheLine *cl = cache->getLine(input_data);
            cl->state = Constants::M_State;
            resetState();
        } else if (state == Constants::M_State) {
            cache_hit++;
            nonSharedData++;
            resetState();
        } else if (state == Constants::Sm_State) {
            cache_hit++;
            sharedData++;
            if (cache->checkOthers(input_data)) {
                bus->putOnBus(BusTransaction::BusUpdateTransaction(input_data));
            } else {
                CacheLine *cl = cache->getLine(input_data);
                cl->state = Constants::M_State;
            }
            resetState();
        } else if (state == Constants::Sc_State) {
            cache_hit++;
            sharedData++;
            if (cache->checkOthers(input_data)) {
                CacheLine *cl = cache->getLine(input_data);
                cl->state = Constants::Sm_State;
                bus->putOnBus(BusTransaction::BusUpdateTransaction(input_data));
            } else {
                CacheLine *cl = cache->getLine(input_data);
                cl->state = Constants::M_State;
            }
            resetState();
        }
    }
    return on_process;
}

void CPU::snoopDragon() {
    if (!bus->currentTransaction || (bus->currentTransaction->type != BusTransaction::ReadShared && bus->
                                     currentTransaction->type != BusTransaction::Update)) {
        return;
    }
    BusTransaction *bt = bus->currentTransaction;
    if (!cache->cacheContains(bt->address)) {
        return;
    }
    CacheLine *cl = cache->getLine(bt->address);
    Constants::MESI_States state = cl->state;
    if (bt->type == BusTransaction::ReadShared) {
        if (state == Constants::E_State) {
            bt->isValid = false;
            bus->putOnBus(BusTransaction::ReadResponseTransaction(bt->address));
            cl->state = Constants::Sc_State;
        } else if (state == Constants::M_State) {
            bt->isValid = false;
            bus->putOnBus(BusTransaction::ReadResponseTransaction(bt->address));
            cl->state = Constants::Sm_State;
        } else if (state == Constants::Sm_State) {
            bt->isValid = false;
            bus->putOnBus(BusTransaction::ReadResponseTransaction(bt->address));
        }
    } else if (bt->type == BusTransaction::Update) {
        if (state == Constants::Sm_State) {
            cl->state = Constants::Sc_State;
        }
    }
}
