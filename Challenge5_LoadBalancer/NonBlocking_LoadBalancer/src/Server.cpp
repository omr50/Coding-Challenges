#include "../include/Server.h"
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include "../include/utils.h"

// fd_set read_set;
// fd_set write_set;
// int max_read_sock = 0;
// int max_write_sock = 0;
// int PORT;

// void (*read_callbacks[MAX_CONN])(int);
// void (*write_callbacks[MAX_CONN])(int);
// std::unordered_map<int, int> socket_mapping;


Server::Server(int PORT) : PORT(PORT) {
    FD_ZERO(&this->read_set);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == 0) {
        perror("Socket Creation Failed!");
        exit(EXIT_FAILURE);
    }
    // stops the "address in use" error when frequently restarting server
    int n = 1;
    if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof (n)) < 0) {
        perror ("SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }
    this->fd = fd;
    set_non_blocking(fd);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Error binding socket!");
        exit(EXIT_FAILURE);
    }

    if (listen(fd, 100) == -1) {
        perror("Listen failed!");
        exit(EXIT_FAILURE);
    }

    this->register_socket_in_select(this->fd, 0);
}

void Server::register_socket_in_select(int fd, bool write, Connection* conn) {
    // set in select fd_set, in callback, and update max read/write sock
    if (write) {
        FD_SET(fd, &this->write_set);
        if (fd != this->fd)
            this->write_connections[fd] = conn; 
        this->max_write_sock = std::max(this->max_write_sock, fd);
    } else {
        FD_SET(fd, &this->read_set);
        if (fd != this->fd)
            this->read_connections[fd] = conn; 
        this->max_read_sock = std::max(this->max_read_sock, fd);
    }

}
// WE WILL NEED A REGISTER FOR READ AND WRITE LATER
// SO WEHN READ FINISHES IT REGISTERS WRITE ON THE 
// SERVER CONNECTION SOCK FOR THE OTHER CALLBACK
// BEFORE DE-REGISTERING ITSELF.

void Server::accept_callback() {
    // once select tells us there is a connection
    // we can try to accept; if no connection, error
    // but don't end the program
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int client_sock = accept(this->fd, (sockaddr*)&addr, &len);     
    if (client_sock == -1) {
        perror("Error accepting connection");
        // don't end server, continue
        return;
    }
    set_non_blocking(client_sock);
    Connection* conn = new Connection(client_sock, this);
    // sometimes no data coming through socket so timer to get rid
    // of the unused heap allocated connections so as not to overconsume memory.
    this->register_socket_in_select(client_sock, 0, conn);
}

void Server::start_server() {
    // register the socket in select for read 
    fd_set tmp_read_set;
    fd_set tmp_write_set;

    while (true) {
        int num_fd_ready;
        int max_sockets = std::max(this->max_read_sock, this->max_write_sock);
        // printf("Started wiht %d max sockets\n", max_sockets);
        tmp_read_set = read_set;
        tmp_write_set = write_set;
        if ((num_fd_ready = select(max_sockets + 1, &tmp_read_set, &tmp_write_set, NULL, NULL)) == -1) {
            perror("Error  in select!");
            exit(EXIT_FAILURE);
        }
        // printf("Got past select!\n");

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // REMBER TO USE the number returned from select num_fd_ready for extiting the loop early

        // For each read set, for each write set, call its corresponding function from the callback array
        for (int i = 0; i < max_sockets + 1; i++) {
            
            // for both client and forwarding socket, this should work.
            if (FD_ISSET(i, &tmp_read_set)) {
                // printf("Socket %d set\n", i);
                if (FD_ISSET(i, &read_set)) {
                    if (i == this->fd) {
                        // server socket 
                        this->accept_callback();
                        printf("Got somehting!!!!\n");
                    } else {
                        // connection
                        Connection::read_callback(this->read_connections[i], i);
                    }
                }
            }

            else if (FD_ISSET(i, &tmp_write_set)) {
                if (FD_ISSET(i, &write_set)) {
                    // connection
                    Connection::read_callback(this->read_connections[i], i);
                }
            }
        }
        // printf("One cycle!\n");
    }
}

// create a new non blocking socket connection to 
// one of the servers using round robin.
int Server::server_connection() {
    // when a server is down, what is the rule?
    // move the responsibility to the next available server?
    // 
}