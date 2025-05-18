#include "../include/Server.h"
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include "../include/utils.h"
#include <algorithm>
#include <sys/resource.h>
#include <unistd.h>
#include <fcntl.h> /* Definition of AT_* constants */
#include <dirent.h>

Server::Server(int PORT) : PORT(PORT)
{

    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    fprintf(stderr, "RLIMIT_NOFILE = %lu\n", rl.rlim_cur);

    for (int i = 0; i < 8; i++)
    {
        server_conn serverc = {8001 + i, true};
        this->server_connections.push_back(serverc);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == 0)
    {
        perror("Socket Creation Failed!");
        exit(EXIT_FAILURE);
    }
    // stops the "address in use" error when frequently restarting server
    int n = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n)) < 0)
    {
        perror("SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }
    this->fd = fd;
    set_non_blocking(fd);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(fd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Error binding socket!");
        exit(EXIT_FAILURE);
    }

    if (listen(fd, MAX_CONN) == -1)
    {
        perror("Listen failed!");
        exit(EXIT_FAILURE);
    }

    this->register_socket_in_poll(this->fd, POLLIN);
}

void Server::register_socket_in_poll(int fd, short events, Connection *conn)
{
    struct pollfd pfd{fd, events, 0};
    pollfds.push_back(pfd);
    fd_to_conn[fd] = conn;
}

void Server::deregister_socket(int fd)
{
    // erase from pollfds vector
    pollfds.erase(
        std::remove_if(pollfds.begin(), pollfds.end(),
                       [&](auto &p)
                       { return p.fd == fd; }),
        pollfds.end());
    fd_to_conn.erase(fd);
}

void Server::accept_callback()
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int client_sock = accept(this->fd, (sockaddr *)&addr, &len);
    if (client_sock == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            printf("No new connections can be made!\n");
        }
        perror("Error accepting connection");
        // don't end server, continue
        return;
    }
    set_non_blocking(client_sock);
    Connection *conn = new Connection(client_sock, this);
    // sometimes no data coming through socket so timer to get rid
    // of the unused heap allocated connections so as not to overconsume memory.
    this->register_socket_in_poll(client_sock, POLLIN, conn);
}

void Server::start_server()
{
    int count = 0;
    while (true)
    {
        int n_ready = poll(pollfds.data(), pollfds.size(), /*timeout=*/-1);
        if (n_ready < 0)
        {
            perror("poll");
            exit(1);
        }

        auto snapshot = pollfds;
        for (auto &p : snapshot)
        {

            if (p.fd == this->fd && (p.revents & POLLIN))
            {
                accept_callback();
                continue;
            }

            if (p.revents & POLLIN)
            {
                auto *conn = fd_to_conn[p.fd];
                if (conn)
                    Connection::read_callback(conn, p.fd);
            }
            if (p.revents & POLLOUT)
            {
                auto *conn = fd_to_conn[p.fd];
                if (conn)
                    Connection::write_callback(conn, p.fd);
            }
        }
    }
}

int Server::server_connection()
{
}

int Server::round_robin()
{
    for (int i = 0; i < 8; i++)
    {

        // if the server is up and running use it, otherwise increment up_next_server
        if (this->server_connections[this->up_next_server].status)
        {
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