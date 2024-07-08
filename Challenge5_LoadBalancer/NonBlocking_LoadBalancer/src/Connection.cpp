#include "../include/Connection.h"
#include <unistd.h>

Connection::Connection(int client_sock, Server* server): client_socket(client_sock), server(server) { }

void Connection::read_callback(Connection* conn, int fd) {
    HTTPObject* http_obj = nullptr;

    // read client
    if (fd == conn->client_socket) {
        http_obj = &(conn->req_http_obj);
    } 
    // read forwarding socket
    else if (fd == conn->forwarding_socket) {
        http_obj = &(conn->res_http_obj);
    } else {
        perror("Wrong Socket being read!");
        exit(EXIT_FAILURE);
    }
    // this shouldn't be de-registered at any time throughout the entire
    // connection because the client can close and write eof at any time.
    int n;
    if ((n = read(fd, conn->data_buffer, sizeof(conn->data_buffer))) > 0) {
        // successful read of n bytes
        try {
            http_obj->parse(conn->data_buffer, n);
        } catch (const char* message) {
            printf("%s\n", message);
            delete conn;
        }
        printf("read %d bytes size %d\n", n, sizeof(conn->data_buffer));
        for (int i = 0; i < n; i++) {
            printf("%c ", conn->data_buffer[i]);
        }
        printf("\n");
        // if end of headers and we updated http obj to hold the curr_content length, if it
        // equals the real content length, then we can end the read callback.
        if (http_obj->end_of_body()) {
            printf("Reached end of body!\n");
            // we have read all of the data, de-register this callback, add the write callback
            // on the forwarding socket. Also indicate read_request is true / finished.
            // not sure we want to remove the read callback yet in case client ends connection
            // we have eof so reading is still necesary
            // this->server->read_connections[this->client_socket] = nullptr;

            // MAYBE DON"T ADD THE WRITE CALLBACK HERE, LET SERVER CREATE SERVER SOCK FIRST
            // !!!!!!!!!!!!!____________________________!!!!!!!!!!!!!!!!!!!!!!!_______________________!!!!!!!!!!!!!!!!!!____________
            // also use the if (fd == client, do this), if (fd == forwarding) do this. (also forwarding set to defualt val to avoid errors)
            conn->server->write_connections[conn->forwarding_socket] = conn;
            printf("Seg fault here?\n");
            FD_SET(conn->forwarding_socket, &conn->server->write_set);
        }

    } else if (n == -1) {
        // error but if eagain then fine
        if (errno == EAGAIN) {
            // return and then handle read later
            return;
        } else{
            // error in connection, end it
            printf("GOT ERRORR DELETING NOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\n");
            delete conn;
        }
    } else if (n == 0) { // 0 is eof (client closed)
        printf("CLIENT DISCONNECTED!\n");
        delete conn;
    } else {
        printf("Unexpected value %d\n", n);
    }
}

void Connection::write_callback(Connection* conn, int fd) {
    // write the http string we got to the write sys call
    // keep track of the num_bytes, and if it equals to str length
    // then we have written the entire thing.

    // setting the http object to the opposite one 
    // (if req, forwarding socket) (if res, client socket)
    HTTPObject* http_obj = nullptr;
    // read client
    if (fd == conn->client_socket) {
        http_obj = &(conn->res_http_obj);
    } 
    // read forwarding socket
    else if (fd == conn->forwarding_socket) {
        http_obj = &(conn->req_http_obj);
    } else {
        perror("Wrong Socket being read!");
        exit(EXIT_FAILURE);
    }   

    // two read and two write. client forward read, and client forward write.
    // in both cases, the read will construct the http_string we have
    // in both cases, the write will take the opposite http_string and
    // write it to the client / server.
    int n;
    if ((n = write(fd, &(http_obj->http_string[0]) + http_obj->sent_bytes, http_obj->http_string.size() - http_obj->sent_bytes)) > 0) {
        // valid data read
        http_obj->sent_bytes += n;
        if (http_obj->sent_bytes == http_obj->http_string.size()) {
            printf("Reached the end of Write Transmission!\n");
            // 
        }
    }

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