#ifndef BUSTRANSACTION_H
#define BUSTRANSACTION_H
#include "Constants.h"

class BusTransaction {
public:
    int type;
    int size;
    bool isValid;
    int address = 0;

    enum TransactionType {
        ReadShared,
        WriteBack,
        ReadExclusive,
        Invalidate,
        ReadResponse
    };


    BusTransaction(int type, int size, bool isValid): type(type), size(size), isValid(isValid){}
    BusTransaction(int type, int size, bool isValid, int address): type(type), size(size), isValid(isValid), address(address){}

    static BusTransaction *WriteBackTransaction() {
        return new BusTransaction(WriteBack, CACHE_BLOCK_SIZE, true);
    }

    static BusTransaction *ReadTransaction(int address) {
        return new BusTransaction(ReadShared, CACHE_BLOCK_SIZE, true, address);
    }

    static BusTransaction *ReadResponseTransaction(int address) {
        return new BusTransaction(ReadResponse, CACHE_BLOCK_SIZE, true, address);
    }

    bool isLast();
    void runBus();
};

#endif //BUSTRANSACTION_H
