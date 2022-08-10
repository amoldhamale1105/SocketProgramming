#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

void error(const char*);

int main(int argc, char *argv[])
{
    int sockfd, numBytes;
    struct sockaddr_un serverAddr;
    char buffer[100];

    /* Check if required number of args are provided to the program */
    if (argc < 2){
        fprintf(stderr, "usage: %s <socket_path>\n", argv[0]);
        exit(1);
    }

    /* Initialize struct memory to zero and then populate the server family and path in sockaddr_in struct */
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, argv[1]);

    /* Create a Unix socket (AF_UNIX) and get the socket file descriptor */
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        error("ERROR: Failed to create socket");
    
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        error("ERROR: Failed to connect to socket");
    
    /* Once connection succeeds, send a message to the server and get a response from it */
    printf("Please type a message to be sent to the server: ");
    bzero(buffer, 100);
    fgets(buffer, 100, stdin);
    write(sockfd, buffer, strlen(buffer));
    numBytes = read(sockfd, buffer, 99);
    printf("%s\n", buffer);

    close(sockfd);

    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
