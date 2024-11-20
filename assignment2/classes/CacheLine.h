#ifndef CACHELINE_H
#define CACHELINE_H

#include "Constants.h"

class CacheLine {
public:
    bool valid;
    int tag;
    Constants::MESI_States state;

    CacheLine() : valid(false), tag(-1), state(Constants::I_State) {}
    CacheLine(bool valid, int tag, Constants::MESI_States state) : valid(valid), tag(tag), state(state) {}
};


#endif //CACHELINE_H
