#ifndef DRAM_H
#define DRAM_H
#include <cstdint>


class DRAM {
    // uint8_t* memory;   //memory 1bytes
    size_t size;        //size of dram (# of words)

public:
    DRAM(size_t size);
    size_t getSize();
};


#endif //DRAM_H
