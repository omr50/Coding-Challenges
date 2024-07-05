#pragma once

/*
Non blocking server.
- create server socket
- make it non blocking
- create fd_sets fields for read / write 
- as well as callback arrays

*/
#include <unistd.h>
#include <sys/types.h>
#include <unordered_map>
#include "../include/Connection.h"

#define MAX_CONN 1000
class Server {
    public: 
        fd_set read_set;
        fd_set write_set;
        int max_read_sock = 0;
        int max_write_sock = 0;
        int PORT;
        int fd;

        // read callback array
        void (Connection::*read_callbacks[MAX_CONN])(int);
        // write callback array
        void (Connection::*write_callbacks[MAX_CONN])(int);
        // maintains a map of the original client sender socket and the socket that receives from the server.
        std::unordered_map<int, int> socket_mapping;


        Server(int PORT);
        int initialize_server(int PORT);
        void register_socket_in_select();
        void start_server(int fd);
        void accept_callback();
};