#ifndef CACHE_H
#define CACHE_H
#include <list>
#include <vector>

#include "Bus.h"
#include "CacheLine.h"


class Cache {
    int cacheSize;      // Total cache size in bytes
    int blockSize;      // Size of each cache block in bytes
    int associativity;  // Associativity (n-way set-associative)
    int numSets;        // Total number of sets


public:
    std::vector<std::list<CacheLine>> sets;  // List of sets, each containing cache lines
    // Constructor
    Cache(int cacheSize, int blockSize, int associativity);
    bool access(int address, Bus *bus, int label);

    bool cacheContains(int address);

    Constants::MESI_States getNewState(Constants::MESI_States oldState, bool isRead, int address);

    Constants::MESI_States getState(int address);

    void addLine(int address, CacheLine cache_line, Bus *bus);

    CacheLine* getLine(int address);

    void removeLine(int address);

    bool set_hit_or_not = false;
    bool hit = false;

    int calculateTag(int address);
    int calculateBlockIdx(int address);
};



#endif //CACHE_H
