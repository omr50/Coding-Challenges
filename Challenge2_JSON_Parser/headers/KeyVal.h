#ifndef KEYVAL_H
#define KEYVAL_H

#include <string>
#include "val.h"


class KeyVal {

public:
    KeyVal(std::string key, val value);
    std::string key;
    val value;
};



#endif //KEYVAL_H
