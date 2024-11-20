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
        return new BusTransaction(WriteBack, 2*(CACHE_BLOCK_SIZE / WORD_SIZE), true);
    }

    static BusTransaction *ReadTransaction(int address) {
        return new BusTransaction(ReadShared, 1, true, address);
    }

    static BusTransaction *ReadResponseTransaction(int address) {
        return new BusTransaction(ReadResponse, 2*(CACHE_BLOCK_SIZE / WORD_SIZE), true, address);
    }

    bool isLast();
    void runBus();
};

#endif //BUSTRANSACTION_H