#include "../include/Connection.h"
#include <unistd.h>

Connection::Connection(int client_sock, Server* server): client_socket(client_sock), server(server) { }

void Connection::read_callback(Connection* conn, int fd) {
    if (fd == conn->client_socket) {
        conn->read_client();
    } else if (fd == conn->forwarding_socket) {
        conn->read_forwarding();
    } else {
        perror("Wrong Socket being read!");
        exit(EXIT_FAILURE);
    }
}


void Connection::read_client() {
    // request
        int n;
        if ((n = read(this->client_socket, this->data_buffer, sizeof(this->data_buffer))) > 0) {
            // successful read of n bytes
            try {
                this->req_http_obj.parse(this->read_request, this->data_buffer, n);
            } catch (const char* message) {
                printf("%s\n", message);
                delete this;
            }
            printf("read %d bytes size %d\n", n, sizeof(this->data_buffer));
            for (int i = 0; i < n; i++) {
                printf("%c ", data_buffer[i]);
            }
            printf("\n");
            // if end of headers and we updated httpobj to hold the curr_content length, if it
            // equals the real content length, then we can end the read callback.
            if (this->req_http_obj.end_of_body()) {
                printf("Reached end of body!\n");
                // we have read all of the data, de-register this callback, add the write callback
                // on the forwarding socket. Also indicate read_request is true / finished.
                this->read_request = true;
                // not sure we want to remove the read callback yet in case client ends connection
                // we have eof so reading is still necesary
                // this->server->read_connections[this->client_socket] = nullptr;
                this->server->write_connections[this->forwarding_socket] = this;
            }

        } else if (n == -1) {
            // error but if eagain then fine
            if (errno == EAGAIN) {
                // return and then handle read later
                return;
            } else{
                // error in connection, end it
                printf("GOT ERRORR DELETING NOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\n");
                delete this;
            }
        } else if (n == 0) { // 0 is eof (client closed)
            printf("CLIENT DISCONNECTED!\n");
            delete this;
        } else {
            printf("Unexpected value %d\n", n);
        }
}

void Connection::read_forwarding() {

}

Connection::~Connection() {
    // get rid of callbacks, close sockets, then memory will be deleted. 
    this->server->read_connections[this->client_socket] = nullptr;    
    this->server->read_connections[this->forwarding_socket] = nullptr;    
    this->server->write_connections[this->client_socket] = nullptr;    
    this->server->write_connections[this->forwarding_socket] = nullptr;    

    FD_CLR(this->client_socket, &server->read_set);
    FD_CLR(this->forwarding_socket, &server->read_set);

    FD_CLR(this->client_socket, &server->write_set);
    FD_CLR(this->forwarding_socket, &server->write_set);

    close(this->client_socket);
    close(this->forwarding_socket);
}