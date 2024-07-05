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

            // register the server socket in read fd_set
            this->register_socket_in_select();
            // server is successfully listening now.
        }

        void Server::register_socket_in_select() {
            FD_SET(this->fd, &this->read_set);
        }

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
            // if accept happens to be blocking, the return
            // above is valid, it will be handled later when
            // there is a connection. If there is a connection
            // don't remove the accept from fd_set for read
            // because we still can accept other connections.

            // if we get a socket back we will construct a long lived
            // connection object on the heap and then use that to further
            // finish the connection. That connection will first be read 
            // asynchronously using a callback. Then we create a new non_blocking
            // socket to connect to the server and that will write in a non_blocking
            // way, then we will read from that socket asynchronously, then we will
            // take that response and then write it to the original socket in a non
            // blocking fashion. 

            // connection conn = new connection(.....);
            // then register read callback and other stuff
            // maybe we don't even need to save the conneciton as a variable
            // and in an array. Since we do register callbacks. So those callbacks
            // can delete the object from within the function if there is an error
            // or if we successefully handled all the data.
        }

        void Server::start_server(int fd) {
            // register the socket in select for read 
            fd_set tmp_read_set = read_set;
            fd_set tmp_write_set = write_set;

            while (true) {
                int num_fd_ready;
                int max_sockets = std::max(max_read_sock, max_write_sock);
                if ((num_fd_ready = select(max_sockets, &tmp_read_set, &tmp_write_set, NULL, NULL)) == -1) {
                    perror("Error  in select!");
                    exit(EXIT_FAILURE);
                }

                // For each read set, for each write set, call its corresponding function from the callback array
                for (int i = 0; i < max_sockets; i++) {
                    
                    // check if the socket is of interest in the returned set
                    // IMPORTANT: Also check if still selected in the regular read_fd, write_fd

                    if (FD_ISSET(i, &tmp_read_set)) {
                        if (FD_ISSET(i, &read_set)) {
                            // a new connection, callback will call accept syscall
                            // then 
                            this->accept_callback();
                        }
                    }
                }


            }
        }