//
// Created by vboxuser on 5/25/24.
//

#include "../headers/Object.h"

#include <algorithm>
#include <stdexcept>

Object::Object() = default;

void Object::addEntry(KeyVal entry) {
   this->entries.push_back(entry);
}


val& Object::operator[](const std::string& key) {
    // search the array member for the key and if it
    // exists return the value. The reference return type
    // will handle returning the reference.

    for (auto& entry : this->entries){
        if (entry.key == key) {
            return entry.value;
        }
    }

    throw std::out_of_range("The value " + key + " was not found!");
}

void Object::print() {
    // for each key print it, but for each value
    // let the value print itself.
    printf("{\n");
    for (auto entry : this->entries) {
        printf("%s : ", entry.key.c_str());
        entry.value.print();
    }
    printf("}\n");
}

