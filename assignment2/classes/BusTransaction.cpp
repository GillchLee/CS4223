#include "BusTransaction.h"



bool BusTransaction::isLast() {
    return size == 1;
}