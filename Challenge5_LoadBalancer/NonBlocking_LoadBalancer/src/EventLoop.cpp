/*
This event loop can run in one thread, or in multiple
The state is separate, and the event loop receives the
sockets from the main thread using round robin and then
The fd_set, and active connections are encapsulated within
this object so that the threads do not have shared state. 
*/
#include "../include/EventLoop.h"
#include "../include/Server.h"
#include "../include/utils.h"
#include <sys/time.h>
#include <sched.h>


EventLoop::EventLoop(Server* server, int fd) : server(server), fd(fd) {
    // atomic variable to store number of connections.
    this->num_connections.store(0);
    if (fd != -1)
        FD_SET(fd, &this->read_set);
    static int thread_counter = 0;
    eventloop_id = thread_counter;
    printf("Event loop id is %d\n", eventloop_id);
    thread_counter++;
}


            //   struct timeval {
            //       time_t      tv_sec;  /* Seconds */
            //       suseconds_t tv_usec; /* Microseconds */
            //   };

void EventLoop::start_loop() {
    // register the socket in select for read 
    fd_set tmp_read_set;
    fd_set tmp_write_set;

    while (true) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        int num_fd_ready;
        tmp_read_set = read_set;
        tmp_write_set = write_set;
        int total_connections = 0;
        int cpu = sched_getcpu();
        printf("Thread is running on CPU core: %d\n", cpu);
        // for (int i = 0; i < FD_SETSIZE + 1; i++) {
        //     if (FD_ISSET(i, &read_set))
        //         total_connections++;
        //     if (FD_ISSET(i, &write_set))
        //         total_connections++;
        // }

        // printf("thread id %d is processing\n", this->eventloop_id);
        if ((num_fd_ready = select(FD_SETSIZE, &tmp_read_set, &tmp_write_set, NULL, &tv)) == -1) {
            perror("Error  in select!");
            exit(EXIT_FAILURE);
        }
        int read_count = 0, write_count = 0, running_count = 0;
        if (this->num_connections) {
            if (this->is_main && this->server->threads_enabled) {
                if (FD_ISSET(this->fd, &tmp_read_set) && FD_ISSET(this->fd, &read_set))
                        // printf("thread id %d is accepting main\n", this->eventloop_id);
                        this->accept_callback();
            }
            else {
                for (int i = 0; i < FD_SETSIZE + 1; i++) {
                    if (FD_ISSET(i, &tmp_read_set) && FD_ISSET(i, &read_set)) {
                        if (i == this->fd) {
                            // server socket incoming connection 
                            // printf("thread id %d is accepting\n", this->eventloop_id);
                            this->accept_callback();
                        } else {
                            // read callback 
                            Connection::read_callback(this->read_connections[i], i);
                            // printf("thread id %d is reading\n", this->eventloop_id);
                        }
                    }

                    else if (FD_ISSET(i, &tmp_write_set) && FD_ISSET(i, &write_set)) {
                        // write callback 
                        // printf("thread id %d is writing\n", this->eventloop_id);
                        Connection::write_callback(this->write_connections[i], i);
                    }
                }
            } 
        }
    }
}


void EventLoop::register_socket_in_select(int fd, bool write, Connection* conn) {
    // set in select fd_set, in callback, and update max read/write sock
    if (write) {
        FD_SET(fd, &this->write_set);
        if (fd != this->fd)
            this->write_connections[fd] = conn; 
    } else {
        FD_SET(fd, &this->read_set);
        if (fd != this->fd)
            this->read_connections[fd] = conn; 
    }

}


void EventLoop::accept_callback() {
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
    // sometimes no data coming through socket so timer to get rid
    // of the unused heap allocated connections so as not to overconsume memory.
    if (this->server->threads_enabled) {
        int min_thread;
        int min_conn = 1000000;
        int curr_num_conn;
        for (int i = 0; i < NUM_THREADS; i++) {
            curr_num_conn = this->server->event_loops[i]->num_connections; 
            if (min_conn < curr_num_conn) {
                min_thread = i;
                min_conn = curr_num_conn;
            }
        }
        
        Connection* conn = new Connection(client_sock, this->server, this->server->event_loops[curr_num_conn]);
        this->server->event_loops[curr_num_conn]->register_socket_in_select(client_sock, 0, conn);
        // increment number of connections using atomic variable.
        this->server->event_loops[curr_num_conn]->num_connections++;
    }
    else {
        Connection* conn = new Connection(client_sock, this->server, this->server->main_event_loop);
        this->server->main_event_loop->register_socket_in_select(client_sock, 0, conn);
        this->server->main_event_loop->num_connections++;
    }

}