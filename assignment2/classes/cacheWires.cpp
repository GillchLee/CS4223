//
// Created by Diana on 11/21/2024.
//

#include "cacheWires.h"

bool cacheWires::checkAllCaches(int address) {
    return (cache1->cacheContains(address) || cache2->cacheContains(address) || cache3->cacheContains(address) || cache4
            ->cacheContains(address));
}

bool cacheWires::checkAllOthers(int address, Cache * cache) {
    if (cache != cache1 && cache1->cacheContains(address)) {
        return true;
    }
    if (cache != cache2 && cache2->cacheContains(address)) {
        return true;
    }
    if (cache != cache3 && cache3->cacheContains(address)) {
        return true;
    }
    if (cache != cache4 && cache4->cacheContains(address)) {
        return true;
    }
    return false;
}
