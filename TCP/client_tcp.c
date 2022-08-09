#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char*);

int main(int argc, char *argv[])
{
    int sockfd, port, numBytes;
    struct sockaddr_in serverAddr;
    struct hostent *server;
    char buffer[256];
    
    /* Check if required number of args are provided to the program */
    if (argc < 3) {
       fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
       exit(1);
    }
    port = atoi(argv[2]);

    /* Create an internet socket (AF_INET) with TCP protocol and get the socket file descriptor */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR: Failed to open socket");
    
    /* Get server information and extract it to the socketaddr_in server structure */
    server = gethostbyname(argv[1]);
    if (server == NULL){
        fprintf(stderr, "ERROR: No host with the given hostname exists\n");
        exit(1);
    }
    
    /* Initialize struct memory to zero and then copy each byte of host address to our sockaddr_in struct */
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char*)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(port);
    
    if (connect(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) 
        error("ERROR: Failed to connect to the server");
    
    /* Once connection succeeds, send a message to the server and get a response from it */
    printf("Please type a short message to be sent to the server: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    numBytes = write(sockfd, buffer, strlen(buffer));
    if (numBytes < 0)
         error("ERROR: Failed to write to socket");

    /* Reset the buffer to zeros and prepare it to receive a response from the server */
    bzero(buffer, 256);
    numBytes = read(sockfd, buffer, 255);
    if (numBytes < 0) 
         error("ERROR: Failed to read from socket");
    printf("%s\n", buffer);
    
    close(sockfd);
    
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
