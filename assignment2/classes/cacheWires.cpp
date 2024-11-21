//
// Created by Diana on 11/21/2024.
//

#include "cacheWires.h"

bool cacheWires::checkAllCaches(int address) {
    return (cache1->cacheContains(address) || cache2->cacheContains(address) || cache3->cacheContains(address) || cache4
            ->cacheContains(address));
}
