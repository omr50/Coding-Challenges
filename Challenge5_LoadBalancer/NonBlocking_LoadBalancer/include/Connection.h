#pragma once
/*
    if we get a socket back we will construct a long lived
    connection object on the heap and then use that to further
    finish the connection. That connection will first be read 
    asynchronously using a callback. Then we create a new non_blocking
    socket to connect to the server and that will write in a non_blocking
    way, then we will read from that socket asynchronously, then we will
    take that response and then write it to the original socket in a non
    blocking fashion. 


    - so our connection object should have two fds, one for the initial client
    - and one for the socket forwarding to server.

*/

#include <string>
#include "./HTTPObject.h"
#include "./Server.h"

class Server;

class Connection {
    public:
        // read -> write -> read
        int client_socket;
        // if the operations in the forwarding socket
        // fail, we need to close the client socket as
        // well.
        int forwarding_socket;
        int timer;
        char data_buffer[64];
        // helps parse request http
        // and stores it, and also
        // stores http response when sending
        // it back to original client.
        HTTPObject req_http_obj = HTTPObject();
        HTTPObject res_http_obj = HTTPObject();
        Server* server = nullptr;
        // HTTPParser parser;
        // HTTPObj    http_obj;

        // read callback handles both sock internally
        // or 2 diff functions called within it 

        // what structures hold the data?
        // This will probably change later as we
        // figure out the http parser. So we will
        // have some sort of buffer structure that will
        // hold the data and append it to some sort of permanent
        // string. Keep reading until eof or some sort of timeout
        // or more logic when we find out more about handling http.
        Connection(int client_sock, Server* server);
        
        static void read_callback(Connection* conn, int fd);
        static void write_callback(Connection* conn, int fd);

        
        ~Connection();
};