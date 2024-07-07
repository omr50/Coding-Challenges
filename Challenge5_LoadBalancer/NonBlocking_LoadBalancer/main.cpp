#include <iostream>
#include "./include/Server.h" 
#include "./include/Connection.h" 

int main() {


    Server server(8080);

    server.start_server();
    return 0;
}