#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void error(const char*);

int main(int argc, char *argv[])
{
    int sockfd, numBytes;
    socklen_t len;
    struct sockaddr_in serverAddr, clientAddr;
    struct hostent *server;
    char buffer[256];

    /* Check if required number of args are provided to the program */
    if (argc < 3){
        fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    /* Create an internet socket (AF_INET) with UDP protocol and get the socket file descriptor */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR: Failed to open socket");

    /* Get server information and extract it to the socketaddr_in server structure */
    serverAddr.sin_family = AF_INET;
    server = gethostbyname(argv[1]);
    if (server == NULL){
        fprintf(stderr, "ERROR: No host with the given hostname exists\n");
        exit(1);
    }

    bcopy((char*)server->h_addr, (char*)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(atoi(argv[2]));
    len = sizeof(struct sockaddr_in);
    
    /* Start async send and receive with the server over UDP. The server is expected to send and ack to the client */
    printf("Please type a message to be sent to the server: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    
    numBytes = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr*)&serverAddr, len);
    if (numBytes < 0)
        error("ERROR: Failed to send message");
    numBytes = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr*)&serverAddr, &len);
    if (numBytes < 0)
        error("ERROR: Failed to receive data from server");
    printf("%s\n", buffer);
    
    close(sockfd);
    
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
