#include "Cache.h"

#include "Bus.h"
// #include "Constants.h"

Cache::Cache(int cacheSize, int blockSize, int associativity)
    : cacheSize(cacheSize), blockSize(blockSize), associativity(associativity) {

    // Calculate the number of sets
    numSets = (cacheSize / blockSize) / associativity;

    // Initialize each set with an empty list of cache lines
    sets.resize(numSets);
}

bool Cache::access(int address) {
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

    //TODO: on eviction, do a writeback(drive bus and send to memory). Only needs to block until done driving bus
    // Unsure if the CPU needs to block or not on a writeback, or if it just needs to take up the bus
    // Handle a cache miss
    if (set.size() >= associativity) {
        // If the set is full, remove the least recently used (LRU) cache line
        set.pop_back();
        const int WORD_SIZE = 4;
        int BANDWIDTH_PENALTY = 2 * (blockSize / WORD_SIZE);
        busTraffic = busTraffic + (BANDWIDTH_PENALTY/2);
    }

    // Add a new cache line with the correct tag
    CacheLine newLine;
    newLine.valid = true;
    newLine.tag = tag;
    set.push_front(newLine);

    return false;  // Cache miss
}
