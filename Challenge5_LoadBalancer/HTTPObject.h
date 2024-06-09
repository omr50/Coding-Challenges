//
// Created by vboxuser on 6/1/24.
//

#ifndef HTTPOBJECT_H
#define HTTPOBJECT_H
#include <string>


class HTTPObject {

public:
    std::string http_string;

    HTTPObject(std::string http_string);

    void parse();

    int static get_content_length(std::string http_headers);

    void create_response() {

    }
};



#endif //HTTPOBJECT_H
