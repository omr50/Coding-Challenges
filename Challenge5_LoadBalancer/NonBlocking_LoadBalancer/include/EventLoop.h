#pragma once 

#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <unordered_map>
#include <netinet/in.h>
#include <thread>
#include <queue>
#include <mutex> 
#include <functional>
#include <condition_variable>
#include <atomic>
#include "./server_conn.h"
 
class Connection;
class Server;
#define MAX_CONN 1000
#define NUM_THREADS 2
class EventLoop {

    public:
        int fd;
        std::vector<server_conn> server_connections;
        fd_set read_set;
        fd_set write_set;
        Connection* read_connections[MAX_CONN];
        Connection* write_connections[MAX_CONN];
        Server* server = nullptr;
        std::atomic<int> num_connections;
        bool is_main = false;
        int eventloop_id;

        void start_loop();
        void register_socket_in_select(int fd, bool write, Connection* conn);
        EventLoop(Server* server, int fd = -1);
        void accept_callback(); 
};