#include <condition_variable>
#include <cstring>
#include <iostream>
#include <queue>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "HTTPObject.h"

std::queue<int> task_queue;
std::mutex queue_mutex;
std::mutex print_mutex;
std::condition_variable condition;

int counter = 0;

void start_socket_server(int PORT);
void handle_response();


int main() {
    // reset log file for now
    FILE* fptr = fopen("file.txt", "wb");
    fclose(fptr);

    start_socket_server(8001);
    return 0;
}


void start_socket_server(int PORT) {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed!\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed!\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server successfully started on PORT: %d\n", PORT);
    std::vector<std::thread> threads(10);

    for (auto& thread : threads) {
        thread = std::thread(handle_response);
    }
    while (true) {
        socklen_t addrlen = sizeof(address);
        int client_socket = accept(server_fd, (sockaddr*)&address, &addrlen);
        // get the address of the socket inside of the address object.
        std::string ip = "";
        std::string port = std::to_string(ntohs(address.sin_port)); // uint_16 so short
        for (int i = 0; i < 4; i++) {
            int shamt = (24 - (i * 8));
            ip += std::to_string((((0xff << shamt)) & ntohl(address.sin_addr.s_addr)) >> shamt) + ((i == 3) ? "" : ".");
        }
    
        // test
        printf("Clients address: %s and port: %s\n", ip.c_str(), port.c_str()); // in_addr_t is uint32
        if (client_socket < 0) {
            perror("accept failed!\n");
            continue;
        }

        // incoming client should be given to a thread to handle.
        // add to queue and notify one of the threads to wake up.
        std::unique_lock<std::mutex> lock(queue_mutex);
        task_queue.push(client_socket);
        lock.unlock();
        condition.notify_one();
    }

}

// worker thread that will only wake when queue
// is non empty and handle a task.
void handle_response() {
    while (true) {
        // tries to acquire lock
        std::unique_lock<std::mutex> lock(queue_mutex);
        // make sure lock is acquired before queue is checked,
        // then if predicate (queue is not empty) is false we
        // will return to wait state and give up lock.
        condition.wait(lock, [] {return !task_queue.empty();});

        // if we reached this point, it means that we've acquired
        // lock and queue is not empty. So we take a task from queue.

        int client_fd = task_queue.front();
        task_queue.pop();
        lock.unlock();
        counter++;




        FILE* fptr = fopen("file.txt", "ab");
        fprintf(fptr, "Processing task #%d\n", counter);

        static int counter = 0;
        {
            std::lock_guard<std::mutex> lock(print_mutex);
            counter++;
            printf("Processing task #%d\n", counter);
            fflush(stdout);
        }

        char buffer[2048] = {0};
        int total_bytes_received = 0, bytes_received, content_length = -1;
        std::string http_string = "";

        while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
            auto partial_data = std::string(buffer, bytes_received);
            http_string += partial_data;
            total_bytes_received += bytes_received;
            int end_of_headers = http_string.find("\r\n\r\n");

            // Check for the end of headers
            if (end_of_headers != std::string::npos) {
                content_length = HTTPObject::get_content_length(http_string);
                printf("Content length: %d\n", content_length);

                // Calculate remaining bytes after headers
                int header_size = end_of_headers + 4;
                int body_bytes_received = total_bytes_received - header_size;

                // If we have already received more than just the headers, adjust total_bytes_received
                total_bytes_received = body_bytes_received;
                break;
            }
        }

        // Error handling for recv
        if (bytes_received < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        // If content length is specified, read until we receive the complete body
        while (total_bytes_received < content_length) {
            bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

            if (bytes_received < 0) {
                perror("Error in receiving bytes!\n");
                exit(EXIT_FAILURE);
            }

            total_bytes_received += bytes_received;
            auto partial_data = std::string(buffer, bytes_received);
            http_string += partial_data;
        }
        
        // printf("Total bytes received after headers %d should equal to %d\n", total_bytes_received, content_length);
        fputs(http_string.c_str(), fptr);
        fclose(fptr);

        HTTPObject http_obj(http_string);
        http_obj.parse();
    }

        // Receiving data
        // while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        //     total_bytes_received += bytes_received;
        //     auto partial_data = std::string(buffer, bytes_received);
        //     http_string += partial_data;
        //     int end_of_headers = http_string.find("\r\n\r\n");
        //
        //     // Check for the end of headers
        //     if (end_of_headers != std::string::npos) {
        //         // since we have all headers now, we can check for content-length with
        //         // find. If we have content length, then we have body, if we have body, we
        //         // want to read exactly however many bytes it says before exiting the recv.
        //         content_length = HTTPObject::get_content_length(http_string);
        //         printf("Content length: %d\n", content_length);
        //         int header_size = end_of_headers + 4;
        //         total_bytes_received = total_bytes_received - header_size;
        //
        //         printf("All bytes received so far after headers: %d\n", total_bytes_received);
        //         break;
        //     }
        // }
        //
        // // Error handling for recv
        // if (bytes_received < 0) {
        //     perror("recv");
        //     return;
        // }
        //
        // // reset buffer
        // memset(buffer, 0, sizeof(buffer));
        // if (content_length != -1)
        // while (total_bytes_received < content_length) {
        //     bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        //
        //     printf("Got more bytes?\n");
        //     if (bytes_received < 0) {
        //         perror("Error in receiving bytes!\n");
        //         exit(EXIT_FAILURE);
        //     }
        //     total_bytes_received += bytes_received;
        //     auto partial_data = std::string(buffer, bytes_received);
        //     http_string += partial_data;
        // }
        //
        // printf("Total bytes received after headers %d should equal to %d\n", total_bytes_received, content_length);
        // fprintf(fptr, "HTTP STRING: %s\n", http_string.c_str());
        // fclose(fptr);
        //
        // HTTPObject http_obj(http_string);
        // http_obj.parse();


        // Parse and process client request and send a response.


    }
