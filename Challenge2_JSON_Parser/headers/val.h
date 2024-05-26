//
// Created by vboxuser on 5/25/24.
//

#ifndef VAL_H
#define VAL_H

enum TYPE {
    NUMBER,
    BOOLEAN,
    STRING,
    _NULL,
    OBJECT, // a vector or array of key-vals
    ARRAY,   // a vector or array of vals
    OPEN_CURLY,
    CLOSE_CURLY,
    OPEN_SQUARE,
    CLOSE_SQUARE
};

class val {

public:
    void* data;
    TYPE type;
    val(void* data, TYPE type);
    void print();
};



#endif //VAL_H
