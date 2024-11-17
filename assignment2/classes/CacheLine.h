//
// Created by Diana on 11/17/2024.
//

#ifndef CACHELINE_H
#define CACHELINE_H



class CacheLine {
public:
    bool valid;
    int tag;
    //int data;   do not need for simulation

    CacheLine() : valid(false), tag(-1) {}
};


#endif //CACHELINE_H
