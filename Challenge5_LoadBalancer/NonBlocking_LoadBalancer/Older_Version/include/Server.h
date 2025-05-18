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
#include <vector>
#include <netinet/in.h>
#include <unordered_map>
#include <poll.h>

class Connection;

struct server_conn
{
    // saddr
    int PORT;
    bool status;
};

#define MAX_CONN 1000
class Server
{
public:
    std::vector<struct pollfd> pollfds;
    std::unordered_map<int, Connection *> fd_to_conn;
    int max_read_sock = 0;
    int max_write_sock = 0;
    int PORT;
    int fd;
    int up_next_server = 0;
    std::vector<server_conn> server_connections;

    // read callback array
    std::unordered_map<int, Connection *> read_connections;
    // write callback array
    std::unordered_map<int, Connection *> write_connections;
    // Connection *read_connections[MAX_CONN];
    // Connection *write_connections[MAX_CONN];

    // maintains a map of the original client sender socket and the socket that receives from the server.
    std::unordered_map<int, int> socket_mapping;

    Server(int PORT);
    void register_socket_in_poll(int fd, short events, Connection *conn = nullptr);
    void deregister_socket(int fd);
    void start_server();
    void accept_callback();
    int server_connection();
    void load_servers();
    void health_check();
    int round_robin();
};