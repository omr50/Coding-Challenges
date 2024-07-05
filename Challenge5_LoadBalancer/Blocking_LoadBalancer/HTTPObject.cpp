//
// Created by vboxuser on 6/1/24.
//

#include "HTTPObject.h"

#include <iostream>
#include <vector>

HTTPObject::HTTPObject(std::string http_string) {
    this->http_string = http_string;
}

void HTTPObject::parse() {
    // take the http string field and derive headers and body.
        // then parse headers, and parse body based on content type.

        // get the first line which has crlf at end
        int end = this->http_string.find("\r\n\r\n");

        if (end == std::string::npos) {
            perror("Invalid HTTP Headers\n");
            exit(EXIT_FAILURE);
        }

        std::string http_headers = this->http_string.substr(0, end);
        std::string http_body = this->http_string.substr(end + 4, std::string::npos);
        printf("HTTP BODY FROM THE CLASS: %s\n", http_body.c_str());
        FILE* fptr = fopen("file.txt", "ab");
        FILE* f2ptr = fopen("file2.txt", "wb");
        fprintf(fptr, "HEADERS: %s", http_headers.c_str());
        fputs(http_body.c_str(), fptr);
        std::vector<std::string> headers;
        while (true) {
            end = http_headers.find("\r\n");
            if (end == std::string::npos) {
                if (http_headers.size() != 0) {
                    headers.push_back(http_headers);
                }
                break;
            }
            headers.push_back(http_headers.substr(0, end));
            http_headers = http_headers.substr(end + 2);
        }

        for (auto header : headers) {
            fprintf(f2ptr, "HEADER: %s\n", header.c_str());
        }
        fclose(fptr);
        fclose(f2ptr);
        // we have space separated http method, path, and http version

        // cut off start that we just parsed
        // temp_http_string = temp_http_string.substr(end, std::string::npos);

}

int HTTPObject::get_content_length(std::string http_headers) {
    std::string delimiter = "Content-Length: ";
    size_t content_length_index = http_headers.find(delimiter);
    if (content_length_index != std::string::npos) {
        // Start after the "Content-Length: " delimiter
        size_t start_index = content_length_index + delimiter.size();
        size_t end_index = start_index;

        // Move end_index forward to find where the numeric part ends
        while (end_index < http_headers.size() && isdigit(http_headers[end_index])) {
            ++end_index;
        }

        // Extract the numeric part as a substring
        std::string content_length_str = http_headers.substr(start_index, end_index - start_index);

        // Convert the numeric part to an integer
        try {
            return std::stoi(content_length_str);
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid Content-Length value: " << content_length_str << std::endl;
            return -1; // Indicate error
        } catch (const std::out_of_range& e) {
            std::cerr << "Content-Length value out of range: " << content_length_str << std::endl;
            return -1; // Indicate error
        }
    }
    return -1; // Content-Length header not found
}
