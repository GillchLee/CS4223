#ifndef BUSTRANSACTION_H
#define BUSTRANSACTION_H
#include "Constants.h"


enum TransactionType {
    ReadShared,
    WriteBack,
    ReadExclusive,
    Invalidate
};

class BusTransaction {
public:
    int type;
    int size;
    bool isValid;


    BusTransaction(int type, int size, bool isValid): type(type), size(size), isValid(isValid){}

    static BusTransaction WriteBackTransaction() {
        return {WriteBack, CACHE_BLOCK_SIZE, true};
    }

    static BusTransaction ReadTransaction() {
        return{ReadShared, CACHE_BLOCK_SIZE, true};
    }

    bool isLast();
    void runBus();
};

#endif //BUSTRANSACTION_H
