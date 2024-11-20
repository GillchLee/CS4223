#include "Cache.h"

#include "Bus.h"
#include "BusTransaction.h"
// #include "Constants.h"

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



bool Cache::access(int address, Bus* bus) {
    int blockIndex = (address / blockSize) % numSets;  // Calculate set index
    int tag = address / (blockSize * numSets);          // Calculate tag

    // Get the list representing the set
    auto& set = sets[blockIndex];

    // Check for a cache hit by searching the set for the tag
    for (auto it = set.begin(); it != set.end(); ++it) {
        if (it->valid && it->tag == tag) {
            // On a cache hit, move the cache line to the front (LRU policy)
            set.splice(set.begin(), set, it);
            return true;  // Cache hit
        }
    }

    if (set.size() >= associativity) {
        // If the set is full, remove the least recently used (LRU) cache line
        set.pop_back();
        //TODO: BE CAREFUL WITH THIS - ENSURES THAT WRITEBACK BLOCKS BY PUTTING ON QUEUE BEFORE READTRANSACTION
        bus->putOnBus(BusTransaction::WriteBackTransaction());
    }

    bus->putOnBus(BusTransaction::ReadTransaction(address));

    //TODO:FIX THIS

    // Add a new cache line with the correct tag
    CacheLine newLine;
    newLine.valid = true;
    newLine.tag = tag;
    set.push_front(newLine);

    return false;  // Cache miss
}
