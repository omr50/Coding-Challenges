#include "../include/Connection.h"
#include "../include/utils.h"
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

Connection::Connection(int client_sock, Server *server) : client_socket(client_sock), server(server) {}

void Connection::read_callback(Connection *conn, int fd)
{
    HTTPObject *http_obj = nullptr;

    // read client
    if (fd == conn->client_socket)
    {
        http_obj = &(conn->req_http_obj);
    }
    // read forwarding socket
    else if (fd == conn->forwarding_socket)
    {
        http_obj = &(conn->res_http_obj);
    }
    else
    {
        perror("Wrong Socket being read!");
        exit(EXIT_FAILURE);
    }
    // this shouldn't be de-registered at any time throughout the entire
    // connection because the client can close and write eof at any time.
    int n;
    if ((n = read(fd, conn->data_buffer, sizeof(conn->data_buffer))) > 0)
    {
        // successful read of n bytes
        try
        {
            http_obj->parse(conn->data_buffer, n);
        }
        catch (const char *message)
        {
            // printf("%s\n", message);
            delete conn;
            return;
        }
        if (http_obj->end_of_body())
        {

            conn->server->deregister_socket(fd);

            if (fd == conn->client_socket)
            {
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd <= 0)
                {
                    // printf("Delete 2\n");
                    delete conn;
                    return;
                }

                if (set_non_blocking(sockfd))
                {
                    conn->forwarding_socket = sockfd;
                }
                else
                {
                    // printf("Delete 3\n");
                    delete conn;
                    return;
                }
                // before we add the socket to select, we have to connect.
                sockaddr_in serv_addr;
                // use round robin to get the correct server and then get the port and ip
                memset(&serv_addr, 0, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                // if no server available through round robin we will crash anyway
                // so no need for error detection.
                serv_addr.sin_port = htons(conn->server->round_robin());
                if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
                {

                    perror("Invalid address / Address not supported");
                    exit(EXIT_FAILURE);
                }
                int res = connect(conn->forwarding_socket, (sockaddr *)&serv_addr, sizeof(serv_addr));
                // printf("Connect went through! %d\n", res);
                if (res == -1)
                {
                    if (errno == EAGAIN)
                    {
                    }
                    // printf("EAGAIN ERROR!\n");
                    else if (errno == EINPROGRESS)
                    {
                    }
                    // printf("EINPROGRESS ERROR!\n");
                    else
                    {
                        // printf("Delete 4\n");
                        delete conn;
                        return;
                    }
                }
                // printf("Testing no error\n");
                conn->server->register_socket_in_poll(conn->forwarding_socket, POLLOUT, conn);
                // printf("SG FAULT HERE?\n");
            }
            else if (fd == conn->forwarding_socket)
            {
                conn->server->register_socket_in_poll(conn->client_socket, POLLOUT, conn);
            }
            else
            {
                printf("Wrong Socket being read!");
            }
        }
    }
    else if (n == -1)
    {
        // error but if eagain then fine
        if (errno == EAGAIN)
        {
            return;
        }
        else
        {
            delete conn;
            return;
        }
    }
    else if (n == 0)
    {
        // eof
        if (http_obj->end_of_body())
        {

            return;
        }
        else
        {
            // printf("Delete 6\n");
            delete conn;
            return;
        }
    }
    else
    {
        printf("Unexpected value %d\n", n);
    }
}

void Connection::write_callback(Connection *conn, int fd)
{
    // write the http string we got to the write sys call
    // keep track of the num_bytes, and if it equals to str length
    // then we have written the entire thing.

    // setting the http object to the opposite one
    // (if req, forwarding socket) (if res, client socket)
    HTTPObject *http_obj = nullptr;
    // read client
    if (fd == conn->client_socket)
    {
        http_obj = &(conn->res_http_obj);
    }
    // read forwarding socket
    else if (fd == conn->forwarding_socket)
    {
        http_obj = &(conn->req_http_obj);
    }
    else
    {
        perror("Wrong Socket being read!");
        exit(EXIT_FAILURE);
    }

    // two read and two write. client forward read, and client forward write.
    // in both cases, the read will construct the http_string we have
    // in both cases, the write will take the opposite http_string and
    // write it to the client / server.
    int n;
    if ((n = write(fd, &(http_obj->http_string[0]) + http_obj->sent_bytes, http_obj->http_string.size() - http_obj->sent_bytes)) > 0)
    {
        // valid data read
        http_obj->sent_bytes += n;
        if (http_obj->sent_bytes == http_obj->http_string.size())
        {
            if (fd == conn->forwarding_socket)
            {
                // clear the current socket
                conn->server->deregister_socket(fd);
                // add the read for the forwarding socket
                conn->server->register_socket_in_poll(conn->forwarding_socket, POLLIN, conn);
            }
            else if (fd == conn->client_socket)
            {
                // destroy connection because we are done sending.
                delete conn;
                return;
            }
        }
    }
}

Connection::~Connection()
{
    if (client_socket >= 0)
    {
        server->deregister_socket(client_socket);
        close(client_socket);
    }
    if (forwarding_socket >= 0)
    {
        server->deregister_socket(forwarding_socket);
        close(forwarding_socket);
    }
}
