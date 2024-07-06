#include "../include/Connection.h"
#include <unistd.h>

void Connection::read_callback(int fd) {
    if (fd == this->client_socket) {
        this->read_client();
    } else if (fd == this->forwarding_socket) {
        this->read_forwarding();
    } else {
        perror("Wrong Socket being read!");
        exit(EXIT_FAILURE);
    }
}


void Connection::read_client() {
    // request
    if (!this->read_request) {
        int n;
        if (n = read(this->client_socket, this->data_buffer, sizeof(data_buffer)) > 0) {
            // successful read of n bytes
            try {
                this->http_obj.parse(data_buffer, n);
            } catch (const char* message) {
                printf("%s\n", message);
                delete this;
            }

            // if end of headers and we updated httpobj to hold the curr_content length, if it
            // equals the real content length, then we can end the read callback.

        } else if (n == -1) {
            // error but if eagain then fine
            if (errno == EAGAIN) {
                // return and then handle read later
                return;
            } else{
                // error in connection, end it
                delete this;
            }
        }
        


    }
    // response
    else {

    }
}

Connection::~Connection() {
    // get rid of callbacks, close sockets, then memory will be deleted. 
    this->server->read_callbacks[this->client_socket] = nullptr;    
    this->server->read_callbacks[this->forwarding_socket] = nullptr;    
    this->server->write_callbacks[this->client_socket] = nullptr;    
    this->server->write_callbacks[this->forwarding_socket] = nullptr;    

    close(this->client_socket);
    close(this->forwarding_socket);
}