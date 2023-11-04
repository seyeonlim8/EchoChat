/**
 * The server side of a Real-Time Server-Client Chat.
 * Created by Seyeon Lim in 2023.
 */

#include "iostream"
#include "string"
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "pthread.h"
#include "cstring"
#include "cstdlib"

void *handleClients(void *arg);

volatile sig_atomic_t keep_running = 1;

void handle_shutdown_signal(int sig) {
    // Set the flag to false so the main loop can exit
    keep_running = 0;
}
int main()
{
    //Will contain message
    int portNum = 12345;
    int buffsize = 1024;
    char buffer[buffsize];
    bool terminate = false;

    //Creating a socket; IPv4 communication, TCP socket.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        std::cout << "Error creating socket." << std::endl;
        //indicate program terminated with error if socket wasn't created.
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket created. - Server" << std::endl;

    //Binding
    //sockaddr_in is a struct used to store endpoint address
    sockaddr_in my;
    //Specifying address family; IPv4.
    my.sin_family = AF_INET;
    //Custom port to reduce conflict. We convert this from Host Byte Order to Network Byte Order (big-endian)
    my.sin_port = htons(portNum);
    //Sets the IP address; since IP addr is unknown, use INADDR_ANY
    my.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr*)&my, sizeof(sockaddr)) < 0) {
        //If it returns -1, it means it failed to bind the socket.
        std::cout << "Error binding socket." << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    //backlog 10; max length of connections queue is 10.
    if(listen(sockfd, 10) < 0) {
        //If it returns -1, it means it failed to listen on the socket.
        std::cout << "Error listening on socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_shutdown_signal);

    while(keep_running) { // keep_running should be a volatile sig_atomic_t flag
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int connection = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if(connection < 0) {
            std::cout << "Error accepting connection." << std::endl;
            continue;
        }

        pthread_t clientThread;
        int* new_conn = new int(connection);
        if(pthread_create(&clientThread, NULL, handleClients, (void*)new_conn) != 0) {
            std::cout << "Failed to create thread for client." << std::endl;
            close(connection);
            delete new_conn; // Make sure to delete if thread creation failed
        } else {
            // Detach the thread so that resources are freed once it finishes
            pthread_detach(clientThread);
        }
    }

// Clean up and close the main listening socket
    close(sockfd);
    std::cout << "Server shut down." << std::endl;
    return 0;
}

void *handleClients(void *arg) {
    int connection = *(int*)arg;
    delete (int*)arg;
    int buffsize = 1024;
    char buffer[buffsize];
    bool terminate = false;

    //must be pointer since we need const void
    const char *startingMsg = "Welcome to the chat server! To exit, type '#'.\n";
    send(connection, startingMsg, strlen(startingMsg), 0);

    while(!terminate) {
        //Receive msg from client. If nothing's received, terminate.
        int bytesRead = recv(connection, buffer, buffsize, 0);
        if(bytesRead <= 0) {
            terminate = true;
            break;
        }

        //Null-terminate to indicate end of msg.
        buffer[bytesRead] = '\0';

        if(strcmp(buffer, "#") == 0) {
            terminate = true;
            break;
        }

        // Print the received message to the server's console
        std::cout << "Client: " << buffer << std::endl;

        // Send a response or message back to the client
        std::cout << "Server: ";
        std::cin.getline(buffer, sizeof(buffer));
        send(connection, buffer, strlen(buffer), 0);
    }

    // Close the client connection and exit the thread
    close(connection);
    pthread_exit(NULL);
}

