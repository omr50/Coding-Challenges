#include "../include/HTTPObject.h"

void HTTPObject::parse(char buffer[], int length) {
    std::string temp(buffer);
    this->http_string += temp;
    int end_of_headers_delimiter = this->http_string.find("\r\n\r\n");
    this->end_of_headers = (end_of_headers_delimiter != std::string::npos);
    if (!this->end_of_headers) {
        // load headers into map
        std::string header_string = this->http_string.substr(0, end_of_headers_delimiter + 2);
        this->parse_headers(header_string);
        this->current_content_length = http_string.size() - end_of_headers_delimiter + 4;
    } 
}


void HTTPObject::parse_headers(std::string header_string) {
    int delimiter_index = 0;
    while ((delimiter_index = header_string.find("\r\n")) != std::string::npos) {
        std::string header = header_string.substr(0, delimiter_index);
        header_string = header_string.substr(delimiter_index + 2, header_string.size());
        int header_delimiter_index = header.find(":");
        if (header_delimiter_index == std::string::npos) {
            throw "Error in HTTP Parsing";
        }
        std::string header_name = header.substr(0, header_delimiter_index); 
        std::string header_val = header.substr(header_delimiter_index+2, header.size());
        this->headers[header_name] = header_val;
        printf("header name: %s, header value: %s\n", header_name.c_str(), header_val.c_str());
    }
}