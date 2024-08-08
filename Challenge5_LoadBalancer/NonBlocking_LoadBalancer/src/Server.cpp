#include "../include/Server.h"
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include "../include/utils.h"
#include <queue>

// fd_set read_set;
// fd_set write_set;
// int max_read_sock = 0;
// int max_write_sock = 0;
// int PORT;

// void (*read_callbacks[MAX_CONN])(int);
// void (*write_callbacks[MAX_CONN])(int);
// std::unordered_map<int, int> socket_mapping;


Server::Server(int PORT) : PORT(PORT) {
    for (int i = 0; i < 8; i++) {
        this->server_connections.push_back({8001 + i, true});
    }

    FD_ZERO(&this->read_set);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0) {
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
    for (int i = 0; i < NUM_THREADS; i++) {
        this->thread_pool.emplace_back(&Server::worker, &task_queue, &q_mutex);
    }
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
                        this->task_queue.push([this] () {this->accept_callback();}); 
                        // printf("Got somehting!!!!\n");
                    } else {
                        // connection
                        this->task_queue.push([this, i] () {Connection::read_callback(this->read_connections[i], i);});
                    }
                }
            }

            else if (FD_ISSET(i, &tmp_write_set)) {
                if (FD_ISSET(i, &write_set)) {
                    // connection
                    this->task_queue.push([this, i] () {Connection::write_callback(this->write_connections[i], i);});
                }
            }
        }
        // printf("One cycle!\n");
    }
}


void Server::health_check() {
    // make a call to each server and see if it responds
    // Should it be regular tcp connection? and should it be
    // blocking or non-blocking? or some kind of ping?


    // how is health check done?
    // Do we create a connection specifically for health check?
    // it can't be persistent can it?
    // http connection?
    // or some sort of ping connection?

    // active health checks:
    // - lb actively sends health checks to the servers.

    // passive health checks:
    // - the lb observes how the servers respond.
    // - Assuming if conn refused or lots of errors
    // - we can determine the server isn't doing good.

    // so the passive checks can see things like errors in the 500s
    // to show that the server is not functioning properly.

    // we can then retry the connection on the next server in the load balancer
    // but we have to add a limit on the retries because a client can have an unintentional
    // or maliciously malformed connection, and so the connection will cause server errors,
    // and then that will cause the connection to keep trying on other servers one by one
    // and then that will cause the connection to stay alive forever.

    // also if we do determine that a server keeps having errors, do we keep its status as down
    // and then we have a timer where we come back to it? or do we do a health check on every loop
    // when the select finishes. Since it seems like doing it on a timer doesn't really matter since
    // it only really matters if we get incoming connections. So we can do it when we have connections.

}
int Server::round_robin() {
    for (int i = 0; i < 8; i++) {

        // if the server is up and running use it, otherwise increment up_next_server
        if (this->server_connections[this->up_next_server].status) {
            // printf("PORT IS %d\n", this->server_connections[this->up_next_server].PORT);
            int return_port = this->server_connections[this->up_next_server].PORT; 
            this->up_next_server++;
            this->up_next_server %= 8;
            return return_port;
        }
        
    }
    // exit if no server is up (no point of keeping the load balancer up)
    perror("No servers are connected!!\n");
    exit(EXIT_FAILURE);
}

void Server::worker(std::queue<std::function<void()>>* q, std::mutex *q_mutex) {
    while (true) {
        q_mutex->lock();
        std::function<void()> task;
        if (!q->empty()) {
            task = q->front();
            q->pop();
        }
        q_mutex->unlock();
        task();
    }
}