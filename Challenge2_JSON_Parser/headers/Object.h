//
// Created by vboxuser on 5/25/24.
//

#ifndef OBJECT_H
#define OBJECT_H
#include <vector>
#include "val.h"
#include "KeyVal.h"


class Object {
protected:
    std::vector<KeyVal> entries;

public:
    Object();
    void addEntry(KeyVal entry);
    val& operator[](const std::string& key);
    void print();
};



#endif //OBJECT_H
