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
#include <condition_variable>
#include <atomic>
#include "./server_conn.h"


class Connection;
class EventLoop;


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
        bool threads_enabled;
        std::condition_variable cv;
        bool running_task[FD_SETSIZE] = {false};
        std::mutex running_tasks_mutex;
        std::vector<std::function<void()>> operations;
        std::mutex operations_mutex;
        std::atomic<int> task_counter;
        std::condition_variable atomic_cv;
        std::mutex atomic_cv_mutex;
        std::vector<EventLoop*> event_loops;
        EventLoop* main_event_loop = nullptr;
        int event_round_robin = 0;



        Server(int PORT, bool threads_enabled);
        void register_socket_in_select(int fd, bool write, Connection* conn = nullptr);
        void start_server();
        static void start_event_loop(EventLoop* eventloop);
        int round_robin();
        void load_servers();
        void health_check();
        // static void worker(std::queue<std::function<void()>>* q, std::mutex* q_mutex, std::condition_variable* cv);
        static void worker(std::queue<std::function<void()>>* q, std::mutex* q_mutex, std::condition_variable* taskq_cv, std::condition_variable* atomic_cv, std::atomic<int>* task_counter);
        void queue_operation(std::function<void()> op); 
        void perform_operations();
};