//
// Created by Diana on 11/21/2024.
//

#ifndef CACHEWIRES_H
#define CACHEWIRES_H
#include "Cache.h"

class Cache;

class cacheWires {
public:
    Cache *cache1;
    Cache *cache2;
    Cache *cache3;
    Cache *cache4;

    bool checkAllCaches(int address);

    bool checkAllOthers(int address, Cache * cache);
};



#endif //CACHEWIRES_H
