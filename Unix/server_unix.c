#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <stdio.h>

void error(const char *);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, numBytes;
    socklen_t clientLen;
    struct sockaddr_un clientAddr, serverAddr;
    char buf[100];

    /* Check if required number of args are provided to the program */
    if (argc < 2){
        fprintf(stderr, "usage: %s <path/to/socket_name>\n", argv[0]);
        exit(1);
    }

    /* Create a Unix socket (AF_UNIX) and get the socket file descriptor */
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        error("ERROR: Failed to create socket");

    /* Initialize struct memory to zero and then populate the server family and path in sockaddr_in struct */
    bzero((char*)&serverAddr, sizeof(serverAddr));
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, argv[1]);

    /* Bind the socket with the given server parameters */
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        error("ERROR: Failed to bind with socket");

    /* If binding succeeds, put the server socket on listen mode to listen to client requests */
    listen(sockfd, 5);

    /* Place a blocking accept call until a client lurks around for this service */
    clientLen = sizeof(clientAddr);
    newsockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen);
    if (newsockfd < 0)
        error("ERROR: Failed to establish connection with client");

    /* Read from and write to the new socket file descriptor retrieved from acceptance of connection with a client */
    bzero(buf, 100);
    numBytes = read(newsockfd, buf, 99);
    printf("A connection has been established\n");
    printf("Here is the message from the client: %s\n", buf);

    const char* response = "Server response: I got your message";
    write(newsockfd, response, strlen(response));
    
    close(newsockfd);
    close(sockfd);
    
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
