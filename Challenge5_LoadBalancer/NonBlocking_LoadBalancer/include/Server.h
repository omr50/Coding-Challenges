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
#include "./Connection.h"

class Connection;

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
        Connection* read_connections[MAX_CONN];
        // write callback array
        Connection* write_connections[MAX_CONN];
        // maintains a map of the original client sender socket and the socket that receives from the server.
        std::unordered_map<int, int> socket_mapping;


        Server(int PORT);
        void register_socket_in_select(int fd, bool write, Connection* conn = nullptr);
        void start_server();
        void accept_callback();
};