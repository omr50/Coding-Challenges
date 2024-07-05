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
        // non blocking request reader.
        // parse the http header to get the
        // content length and keep reading.
        
        // as we get data we read it into request
        // string from buffer. 

        // data that is read is added to string, its stored in the
        // http object. It keeps track of how much lines its parsed
        // and the curent string, and the conent lenght and if it found
        // the content length yet. Once it has, we know how much to keep
        // reading. So the loop has two portions. One for read content
        // length, and one for after.
        int n;
        if (n = read(this->client_socket, this->data_buffer, sizeof(data_buffer)) > 0) {

            // successful read of n bytes
        } else if (n == -1) {
            // error but if eagain then fine
            if (errno == EAGAIN) {
                // return and then handle read later
                return;
            } else{
                // error in connection, end it

                // free the memory of the object, remove the callbacks for the client and server forwarding socket
                // and then close both sockets.
                delete this;
            }
        }
        


    }
    // response
    else {

    }
}

Connection::~Connection() {
    
    close(this->client_socket);
    close(this->forwarding_socket);
}