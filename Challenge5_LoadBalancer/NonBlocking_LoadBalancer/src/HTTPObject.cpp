#include "../include/HTTPObject.h"

void HTTPObject::parse(bool req_finished, char buffer[], int length) {
    // std::string temp(buffer, buffer + length);
    for (int i = 0; i < length; i++)
        this->http_string += buffer[i];

    printf("HTTP STRING IN FUNC\n%s\n", this->http_string.c_str());
    int end_of_headers_delimiter = this->http_string.find("\r\n\r\n");
    // this only runs once, the first time the end of headers is false
    // once we find headers end we run this and it runs no more. Therefore
    // its a bad idea to update the current_content_length in here since this
    // function won't run again, so if we need to update the content on multiple
    // attempts it won't be good.
    if (!this->end_of_headers && end_of_headers_delimiter != std::string::npos) {
        // load headers into map
        this->end_of_headers = true;
        std::string header_string = this->http_string.substr(0, end_of_headers_delimiter + 2);
        this->parse_headers(header_string);
    }
    if (this->end_of_headers) {
        this->current_content_length = http_string.size() - (end_of_headers_delimiter + 4);
    } 
}


void HTTPObject::parse_headers(std::string header_string) {
    int delimiter_index = 0;
    // skip first row since that is not a header it is status
    delimiter_index = header_string.find("\r\n");
    if (delimiter_index == std::string::npos)
        throw "Error: Could not find request line!";
        
    std::string header = header_string.substr(0, delimiter_index);
    printf("request line: %s\n", header.c_str());
    header_string = header_string.substr(delimiter_index + 2, header_string.size());
    while ((delimiter_index = header_string.find("\r\n")) != std::string::npos) {
        header = header_string.substr(0, delimiter_index);
        header_string = header_string.substr(delimiter_index + 2, header_string.size());
        int header_delimiter_index = header.find(":");
        if (header_delimiter_index == std::string::npos) {
            throw "Error in HTTP Parsing";
        }
        std::string header_name = header.substr(0, header_delimiter_index); 
        std::string header_val = header.substr(header_delimiter_index+2, header.size());
        this->headers_map[header_name] = header_val;
        printf("header name: %s, header value: %s\n", header_name.c_str(), header_val.c_str());
    }
}


bool HTTPObject::end_of_body() {
    if (!this->end_of_headers)
        return false;
    printf("end of body call \n");
    printf("http string %s\n", this->http_string.c_str());
    if (this->end_of_headers && this->headers_map.find("Content-Length") == this->headers_map.end())
        return true;
    printf("content length %d\n", std::stoi(this->headers_map["Content-Length"]));
    printf("Current content length: %d\n", this->current_content_length);
    if (this->current_content_length == std::stoi(this->headers_map["Content-Length"])) {
        printf("Content: %s\n", this->http_string.substr(this->http_string.find("\r\n\r\n") + 4, this->http_string.size()).c_str());
        return true;
    }
}