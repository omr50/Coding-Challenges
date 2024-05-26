//
// Created by vboxuser on 5/25/24.
//

#include "../headers/KeyVal.h"


KeyVal::KeyVal(std::string key, val value) : value(std::move(value)){
    this->key = key;
}
