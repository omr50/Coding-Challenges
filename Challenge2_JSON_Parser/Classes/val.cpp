//
// Created by vboxuser on 5/25/24.
//

#include "../headers/val.h"

#include <cstdio>
#include <string>

#include "../headers/Object.h"

val::val(void *data, TYPE type) {
    this->data = data;
    this->type = type;
}

void val::print() {
    switch (this->type) {
        case NUMBER:
            printf("%d,\n", *((int*)this->data));
            break;
        case STRING:
            printf("%s,\n", ((std::string*)this->data)->c_str());
            break;
        case BOOLEAN:
            printf("%d\n", *((bool*)this->data));
            break;
        case _NULL:
            printf("NULL,\n");
            break;
        case OBJECT:
            ((Object*)this->data)->print();
            break;
        case ARRAY:
            // it is actually a vector (easier for size calculation)
            std::vector<val>* arrayPointer = ((std::vector<val>*)this->data);
            std::vector<val>& array = *arrayPointer;
            printf("[\n");
            for (auto& item : array) {
                item.print();
            }
            printf("]\n");
            break;
    }
}

