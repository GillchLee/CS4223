#ifndef CACHE_H
#define CACHE_H
#include <list>
#include <vector>

#include "CacheLine.h"


class Cache {
    int cacheSize;      // Total cache size in bytes
    int blockSize;      // Size of each cache block in bytes
    int associativity;  // Associativity (n-way set-associative)
    int numSets;        // Total number of sets
    std::vector<std::list<CacheLine>> sets;  // List of sets, each containing cache lines


public:
    // Constructor
    Cache(int cacheSize, int blockSize, int associativity);
    bool access(int address);
    bool set_hit_or_not = false;
    bool hit = false;
};



#endif //CACHE_H
