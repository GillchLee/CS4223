#ifndef BUSTRANSACTION_H
#define BUSTRANSACTION_H
#include "Constants.h"


enum TransactionType {
    ReadShared,
    WriteBack,
    ReadExclusive,
    Invalidate,
    ReadResponse
};

class BusTransaction {
public:
    int type;
    int size;
    bool isValid;
    int address = 0;


    BusTransaction(int type, int size, bool isValid): type(type), size(size), isValid(isValid){}
    BusTransaction(int type, int size, bool isValid, int address): type(type), size(size), isValid(isValid), address(address){}

    static BusTransaction WriteBackTransaction() {
        return {WriteBack, CACHE_BLOCK_SIZE, true};
    }

    static BusTransaction ReadTransaction() {
        return{ReadShared, CACHE_BLOCK_SIZE, true};
    }

    static BusTransaction ReadResponseTransaction(int address) {
        return {ReadResponse, CACHE_BLOCK_SIZE, true, address};
    }

    bool isLast();
    void runBus();
};

#endif //BUSTRANSACTION_H
