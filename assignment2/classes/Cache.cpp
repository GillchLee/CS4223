#include "Cache.h"

#include "Bus.h"
#include "BusTransaction.h"

Cache::Cache(int cacheSize, int blockSize, int associativity)
    : cacheSize(cacheSize), blockSize(blockSize), associativity(associativity) {

    // Calculate the number of sets
    numSets = (cacheSize / blockSize) / associativity;

    // Initialize each set with an empty list of cache lines
    sets.resize(numSets);
}

int Cache::calculateTag(int address) {
    int tag = address / (blockSize * numSets);
    return tag;
}

int Cache::calculateBlockIdx(int address) {
    int blockIndex = (address / blockSize) % numSets;
    return blockIndex;
}



bool Cache::access(int address, Bus* bus, int label) {
    int blockIndex = (address / blockSize) % numSets;  // Calculate set index
    int tag = address / (blockSize * numSets);          // Calculate tag

    // Get the list representing the set
    auto& set = sets[blockIndex];

    // Check for a cache hit by searching the set for the tag
    for (auto it = set.begin(); it != set.end(); ++it) {
        if (it->valid && it->tag == tag && it->state != Constants::I_State) {
            // On a cache hit, move the cache line to the front (LRU policy)
            set.splice(set.begin(), set, it);
            return true;  // Cache hit
        }
    }

    if (set.size() >= associativity) {
        // If the set is full, remove the least recently used (LRU) cache line
        set.pop_back();
        bus->putOnBus(BusTransaction::WriteBackTransaction());
    }

    if (label == 1) {
        bus->putOnBus(BusTransaction::ReadXTransaction(address));
    } else {
        bus->putOnBus(BusTransaction::ReadTransaction(address));
    }

    return false;  // Cache miss
}

void Cache::addLine(int address, CacheLine cache_line, Bus *bus) {
    int blockIndex = (address / blockSize) % numSets;  // Calculate set index

    // Get the list representing the set
    auto& set = sets[blockIndex];
    if (set.size() >= associativity) {
        // If the set is full, remove the least recently used (LRU) cache line
        set.pop_back();
        bus->putOnBus(BusTransaction::WriteBackTransaction());
    }
    set.push_back(cache_line);
}

Constants::MESI_States Cache::getState(int address) {
    int blockIndex = (address / blockSize) % numSets;  // Calculate set index
    int tag = address / (blockSize * numSets);          // Calculate tag

    // Get the list representing the set
    auto& set = sets[blockIndex];

    // Check for a cache hit by searching the set for the tag
    for (auto it = set.begin(); it != set.end(); ++it) {
        if (it->valid && it->tag == tag && it->state != Constants::I_State) {
            // On a cache hit, move the cache line to the front (LRU policy)
            set.splice(set.begin(), set, it);
            return it->state;
        }
    }
    return Constants::I_State;
}

Constants::MESI_States Cache::getNewState(Constants::MESI_States oldState, bool isRead) {
    //TODO: add wires so that you know if I read needs to go to E or S
    if (isRead && oldState == Constants::I_State) {
        return Constants::S_State;
    }
    else if (!isRead && oldState == Constants::I_State) {
        return Constants::M_State;
    }
    return Constants::NO_State;
}
