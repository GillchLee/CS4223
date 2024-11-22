#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

    static int CACHE_SIZE = 4096;
    static int CACHE_ASSOC = 2;
    static int CACHE_BLOCK_SIZE = 32;
    static std::string FILE_NAME = "blackscholes_";
    static std::string PROTOCOL = "MESI";
    static int WORD_SIZE = 4;

class Constants {
public:
    enum MESI_States {
        FAKE_STATE,
        M_State,
        E_State,
        S_State,
        I_State,
        Sc_State,
        Sm_State,
        D_State,
        NO_State
    };
};



#endif //CONSTANTS_H
