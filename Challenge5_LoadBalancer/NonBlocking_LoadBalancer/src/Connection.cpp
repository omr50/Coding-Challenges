#include "../include/Connection.h"
#include "../include/utils.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "../include/EventLoop.h"

Connection::Connection(int client_sock, Server *server, EventLoop *eventloop) : client_socket(client_sock), server(server), eventloop(eventloop) {}

void Connection::read_callback(Connection *conn, int fd)
{
    if (conn == nullptr)
    {
        return;
    }

    HTTPObject *http_obj = nullptr;

    // printf("fd = %d and fsock = %d\n", fd, conn->forwarding_socket);
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
        // printf("Read %d bytes\n", n);
        try
        {
            http_obj->parse(conn->data_buffer, n);
        }
        catch (const char *message)
        {
            // printf("Error during parsing: %s\n", message);
            // printf("Delete 1\n");
            conn->thread_safe_delete();
            return;
        }
        // printf("read %d bytes size %d\n", n, sizeof(conn->data_buffer));
        // printf("\n");
        // if end of headers and we updated http obj to hold the curr_content length, if it
        // equals the real content length, then we can end the read callback.
        if (http_obj->end_of_body())
        {
            // printf("Reached end of body!\n");
            // we have read all of the data, de-register this callback, add the write callback
            // on the forwarding socket. Also indicate read_request is true / finished.
            // not sure we want to remove the read callback yet in case client ends connection
            // we have eof so reading is still necesary
            // this->server->read_connections[this->client_socket] = nullptr;

            // MAYBE DON"T ADD THE WRITE CALLBACK HERE, LET SERVER CREATE SERVER SOCK FIRST
            // !!!!!!!!!!!!!____________________________!!!!!!!!!!!!!!!!!!!!!!!_______________________!!!!!!!!!!!!!!!!!!____________
            // also use the if (fd == client, do this), if (fd == forwarding) do this. (also forwarding set to defualt val to avoid errors)
            // conn->server->write_connections[conn->forwarding_socket] = conn;
            // printf("Seg fault here?\n");

            // should we delete the read from fd set and callback for read? because if we finished reading
            // the body then we shouldn't need to read on this socket any more. We don't need to close it
            // because we still want to write to it the final result but we don't need it for reads and can
            // close that end of it. Each socket should read once and write once, so technically you can
            // de-register that end of it from select when you finish with it.
            conn->eventloop->read_connections[fd] = nullptr;
            FD_CLR(fd, &conn->eventloop->read_set);

            if (fd == conn->client_socket)
            {
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd <= 0)
                {
                    // printf("Delete 2\n");
                    conn->thread_safe_delete();
                    return;
                }

                if (set_non_blocking(sockfd))
                {
                    conn->forwarding_socket = sockfd;
                }
                else
                {
                    // printf("Delete 3\n");
                    conn->thread_safe_delete();
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
                        conn->thread_safe_delete();
                        return;
                    }
                }
                // printf("Testing no error\n");
                // once socket has initiated connect, we can add the callback and if it succeeds
                // and becomes writable, select will trigger.
                conn->eventloop->register_socket_in_select(conn->forwarding_socket, true, conn);
                // printf("SG FAULT HERE?\n");
                // FD_SET(conn->forwarding_socket, &conn->server->write_set);
            }
            else if (fd == conn->forwarding_socket)
            {
                conn->eventloop->register_socket_in_select(conn->client_socket, true, conn);
                // FD_SET(conn->client_socket, &conn->server->write_set);
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
            // return and then handle read later

            return;
        }
        else
        {
            // error in connection, end it
            // printf("GOT ERRORR DELETING NOWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW\n");
            // printf("Delete 5\n");

            conn->thread_safe_delete();
            return;
        }
    }
    else if (n == 0)
    {
        // eof
        // printf("HTTP COMPLETE %s\n", http_obj->http_string.c_str());
        if (http_obj->end_of_body())
        {

            return;
        }
        else
        {
            // printf("Delete 6\n");
            conn->thread_safe_delete();
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
    if (conn == nullptr)
    {
        return;
    }
    // write the http string we got to the write sys call
    // keep track of the num_bytes, and if it equals to str length
    // setting the http object to the opposite one
    // (if req, forwarding socket) (if res, client socket)
    // printf("Made it here?\n");
    HTTPObject *http_obj = nullptr;
    // write client
    if (fd == conn->client_socket)
    {
        http_obj = &(conn->res_http_obj);
    }
    // write forwarding socket
    else if (fd == conn->forwarding_socket)
    {
        http_obj = &(conn->req_http_obj);
        // check if the connect succeded after select
        // has indicated that this socket is writable.
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {

            perror("Getsockopt error");
            conn->thread_safe_delete();
            return;
        }
    }
    else
    {

        printf("Wrong Socket being read!");
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
            // printf("Reached the end of Write Transmission, size = %d and http str size = %d!\n", http_obj->sent_bytes, http_obj->http_string.size());
            // if we wrote to the forwarding socket we can remove the socket from the write callback
            // and the fd set.
            if (fd == conn->forwarding_socket)
            {
                // printf("Finsihed writing to forwarding!\n");
                // printf("Total of %d connections successfully served!\n", conn->server->total_connections_succeeded);

                // clear the current socket
                conn->eventloop->write_connections[fd] = nullptr;
                FD_CLR(fd, &conn->eventloop->write_set);
                // add the read for the forwarding socket
                conn->eventloop->register_socket_in_select(conn->forwarding_socket, false, conn);
                // threads enabled
            }
            else if (fd == conn->client_socket)
            {
                // destroy connection because we are done sending.
                conn->thread_safe_delete();
                return;
            }
        }
    }
    else
    {
        // handle invalid writes (and server errors to start doing the passive healt checks)
        printf("TESTING 123\n");
    }
}

void Connection::thread_safe_delete()
{
    // printf("Delete OP CALLED!!!\n");
    delete this;
}

Connection::~Connection()
{
    // get rid of callbacks, close sockets, then memory will be deleted.
    // printf("DELTE OBJECT CALLED !!!!\n");
    // threads
    this->eventloop->read_connections[this->client_socket] = nullptr;
    this->eventloop->read_connections[this->forwarding_socket] = nullptr;
    this->eventloop->write_connections[this->client_socket] = nullptr;
    this->eventloop->write_connections[this->forwarding_socket] = nullptr;
    FD_CLR(this->client_socket, &this->eventloop->read_set);
    FD_CLR(this->forwarding_socket, &this->eventloop->read_set);

    FD_CLR(this->client_socket, &this->eventloop->write_set);
    FD_CLR(this->forwarding_socket, &this->eventloop->write_set);

    close(this->client_socket);
    close(this->forwarding_socket);
    this->eventloop->num_connections--;
}