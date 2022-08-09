#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

void error(const char*);

int main(int argc, char *argv[])
{
    int sockfd, numBytes;
    socklen_t clientLen;
    struct sockaddr_in serverAddr, clientAddr;
    char buf[1024];

    /* Check if required number of args are provided to the program */
    if (argc < 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* Create an internet socket (AF_INET) with UDP protocol and get the socket file descriptor */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR: Failed to open socket");
    
    bzero(&serverAddr, sizeof(serverAddr));

    /* Set the server socketaddr_in members with appropriate values */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));
    
    /* Bind the socket with the given server parameters */
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        error("ERROR: Failed to bind socket");
    clientLen = sizeof(struct sockaddr_in);
    
    /* Receive multiple requests from the same client and respond by sending an ack. Note that this is not a multiclient connection loop */
    while(1)
    {
        numBytes = recvfrom(sockfd, buf, 1024, 0, (struct sockaddr*)&clientAddr, &clientLen);
        if (numBytes < 0)
            error("ERROR: Failed to receive data from client");
        
        printf("Received a datagram from client: %s\n", buf);
        const char* response = "Server response: I got your message";
        numBytes = sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&clientAddr, clientLen);
        if (numBytes < 0)
            error("ERROR: Failed to send data to client");
    }

    close(sockfd);

    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
