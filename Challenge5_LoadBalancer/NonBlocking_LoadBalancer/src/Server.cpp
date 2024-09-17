/*
This is the main server. It will listen for connections and forward them to the event loops.
If there is only one thread then the event loop will be in the main thread as well. If there 
are threads, then each thread will have its own even loop and the server will forward accepted
connections to the event loops.
*/

#include "../include/Server.h"
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include "../include/utils.h"
#include <queue>
#include "../include/EventLoop.h"


Server::Server(int PORT, bool threads_enabled = false) : PORT(PORT), threads_enabled(threads_enabled) {
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
    // FD_SET(fd, &this->read_set);

    if (threads_enabled) {
        printf("testing if thread works\n");
        for (int i = 0; i < NUM_THREADS; i++) {
            this->event_loops.push_back(new EventLoop(this));
        } 
        for (int i = 0; i < NUM_THREADS; i++) {
            this->thread_pool.emplace_back(&Server::start_event_loop, this->event_loops[i]);
        }
        printf("testing if thread works end?\n");
    }
    // always create main event loop anyway
    this->main_event_loop = new EventLoop(this, fd);
    this->main_event_loop->is_main = true;
    this->main_event_loop->num_connections++;
    printf("Server started on port %d\n", PORT);
    printf("Threads are %s\n", threads_enabled ? "enabled" : "disabled");
}


// WE WILL NEED A REGISTER FOR READ AND WRITE LATER
// SO WEHN READ FINISHES IT REGISTERS WRITE ON THE 
// SERVER CONNECTION SOCK FOR THE OTHER CALLBACK
// BEFORE DE-REGISTERING ITSELF.



// event loop per thread. start event loop will run the one particular to the given server
void Server::start_server() {
    printf("STARTING MAIN EVENT LOOP\n");
    this->main_event_loop->start_loop();
}


void Server::start_event_loop(EventLoop* eventloop) {
    eventloop->start_loop();
    
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