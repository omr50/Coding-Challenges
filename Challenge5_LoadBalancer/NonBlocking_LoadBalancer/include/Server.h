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
#include <thread>
#include <queue>
#include <mutex> 
#include <functional>

class Connection;

struct server_conn {
    // saddr 
    int PORT;
    bool status;
};

#define MAX_CONN 1000
#define NUM_THREADS 8 

class Server {
    public: 
        fd_set read_set;
        fd_set write_set;
        int max_read_sock = 0;
        int max_write_sock = 0;
        int PORT;
        int fd;
        int up_next_server = 0;
        std::vector<server_conn> server_connections;
        int total_connections_succeeded = 0;
        std::vector<std::thread> thread_pool;
        std::queue<std::function<void()>> task_queue;
        std::mutex q_mutex;
         
        

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
        int round_robin();
        void load_servers();
        void health_check();
        static void worker(std::queue<std::function<void()>>* q, std::mutex* q_mutex);
};