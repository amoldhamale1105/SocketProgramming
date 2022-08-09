#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char*);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, port, numBytes;
    socklen_t clientLen;
    char buffer[256];
    struct sockaddr_in serverAddr, clientAddr;

    /* Check if required number of args are provided to the program */
    if (argc < 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* Create an internet socket (AF_INET) with TCP protocol and get the socket file descriptor */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR: Failed to open socket");
    
    bzero((char *)&serverAddr, sizeof(serverAddr));
    port = atoi(argv[1]);

    /* Set the server socketaddr_in members with appropriate values */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    /* Bind the socket with the given server parameters */
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        error("ERROR: Failed to bind socket");
    
    /* If binding succeeds, put the server socket on listen mode to listen to client requests */
    listen(sockfd, 5);
    clientLen = sizeof(clientAddr);

    /* Place a blocking accept call until a client lurks around for this service */
    newsockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen);
    if (newsockfd < 0)
        error("ERROR: Failed to accept connection from client");
    
    /* Read from and write to the new socket file descriptor retrieved from acceptance of connection with a client */
    bzero(buffer, 256);
    numBytes = read(newsockfd, buffer, 255);
    if (numBytes < 0)
        error("ERROR: Failed to read from socket");
    
    printf("Here is the message from the client: %s\n", buffer);

    const char* response = "Server response: I got your message";
    numBytes = write(newsockfd, response, strlen(response));
    if (numBytes < 0)
        error("ERROR: Failed to write to socket");
    
    close(newsockfd);
    close(sockfd);
    
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
