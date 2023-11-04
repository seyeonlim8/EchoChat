/**
 * The client side of a Real-Time Server-Client Chat.
 * Created by Seyeon Lim in 2023.
 */

#include "iostream"
#include "string"
#include <sys/types.h>
#include "sys/socket.h"
#include "netinet/in.h"
#include "unistd.h"
#include "arpa/inet.h"

int main()
{
    int portNum = 12345;
    int buffsize = 1024;
    char buffer[buffsize];
    bool terminate = false;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        std::cout << "Error creating socket." << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket created - client." << std::endl;

    sockaddr_in my;
    my.sin_family = AF_INET;
    my.sin_port = htons(portNum);
    //Localhost; use same device
    my.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t len = sizeof(my);
    //Returns 0 upon successful connection.
    if(connect(sockfd, (struct sockaddr*)&my, len) == 0) {
        std::cout << "Connecting to server with port number " << portNum << std::endl;
    }

    std::cout << "Waiting for server to confirm connection." << std::endl;
    recv(sockfd, buffer, buffsize, 0);
    std::cout << "Connection confirmed by server." << std::endl;
    std::cout << "To terminate connection, type '#'." << std::endl;

    do {
        // Clear the buffer.
        memset(buffer, 0, buffsize);

        // Client sends a message.
        std::cout << "Client: ";
        std::string input;
        std::getline(std::cin, input); // Using getline to include spaces.
        strncpy(buffer, input.c_str(), buffsize - 1); // Safely copy to buffer; prevent buffer overflow and keep space for null termination.
        buffer[buffsize - 1] = '\0'; // Ensure null-termination.

        send(sockfd, buffer, strlen(buffer), 0);
        if (buffer[0] == '#') {
            terminate = true;
        }

        if (terminate) {
            break;
        }

        // Now wait for server response.
        std::cout << "Server: ";
        int bytesReceived = recv(sockfd, buffer, buffsize - 1, 0);
        if (bytesReceived <= 0) {
            // Handle errors or connection closed by server.
            break;
        }
        buffer[bytesReceived] = '\0'; // Ensure null-termination.
        std::cout << buffer << std::endl; // Print server's message.
        if (buffer[0] == '#') {
            break; // Terminate if server sends termination character.
        }
    } while (!terminate);


    std::cout << "Connection terminated.";
    close(sockfd);
    return 0;
}
