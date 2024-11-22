#include "Cache.h"

#include "Bus.h"
#include "BusTransaction.h"

Cache::Cache(int cacheSize, int blockSize, int associativity, cacheWires *cw)
    : cacheSize(cacheSize), blockSize(blockSize), associativity(associativity), cacheWire(cw) {

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


CacheLine* Cache::getLine(int address) {
     int blockIndex = (address / blockSize) % numSets;  // Calculate set index
     int tag = address / (blockSize * numSets);          // Calculate tag

     // Get the list representing the set
     auto& set = sets[blockIndex];

     // Check for a cache hit by searching the set for the tag
    for (auto& cacheLine : set) {
        if (cacheLine.tag == tag) {
            return &cacheLine;
        }
    }
}

void Cache::removeLine(int address) {
    int blockIndex = (address / blockSize) % numSets;  // Calculate set index
    int tag = address / (blockSize * numSets);          // Calculate tag

    // Get the list representing the set
    auto& set = sets[blockIndex];

    // Check for a cache hit by searching the set for the tag
    for (auto it = set.begin(); it != set.end(); ++it) {
        if (it->valid && it->tag == tag && it->state != Constants::I_State) {
            set.erase(it);
            return;
        }
    }
}

void Cache::addLine(int address, CacheLine cache_line, Bus *bus) {
    int blockIndex = (address / blockSize) % numSets;  // Calculate set index

    // Get the list representing the set
    auto& set = sets[blockIndex];
    if (set.size() >= associativity) {
        // If the set is full, remove the least recently used (LRU) cache line
        if (set.back().state == Constants::M_State || set.back().state == Constants::Sm_State) {
            bus->putOnBus(BusTransaction::WriteBackTransaction());
        }
        set.pop_back();
    }
    set.push_front(cache_line);
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


Constants::MESI_States Cache::getNewState(Constants::MESI_States oldState, bool isRead, int address, Bus *bus) {
    if (isRead && oldState == Constants::I_State) {
        if (cacheWire->checkAllCaches(address)) {
            return Constants::S_State;
        }
        else {
            return Constants::E_State;
        }
    }
    else if (!isRead && oldState == Constants::I_State) {
        return Constants::M_State;
    }
    return Constants::NO_State;
}


bool Cache::checkOthers(int address) {
    return cacheWire->checkAllOthers(address, this);
}

bool Cache::cacheContains(int address) {
    int blockIndex = (address / blockSize) % numSets;  // Calculate set index
    int tag = address / (blockSize * numSets);          // Calculate tag

    // Get the list representing the set
    auto& set = sets[blockIndex];

    // Check for a cache hit by searching the set for the tag
    for (auto it = set.begin(); it != set.end(); ++it) {
        if (it->valid && it->tag == tag && it->state != Constants::I_State) {
            return true;
        }
    }
    return false;
}
