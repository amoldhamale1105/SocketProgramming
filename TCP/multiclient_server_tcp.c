/**
 * @file multiconnection_server_tcp.c
 * @author Amol Dhamale
 * @brief Multiclient server where a child process is spawned for every new client in order to handle requests independently
 * @version 0.1
 * @date 2022-08-09
 * 
 * @copyright Copyright (c) 2022 Amol Dhamale
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <netinet/in.h>

void dostuff(int);
void error(const char*);
void handle_child_exit();

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, port, pid;
    socklen_t clientLen;
    struct sockaddr_in serverAddr, clientAddr;

    /* Check if required number of args are provided to the program */
    if (argc < 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* Create an internet socket (AF_INET) with TCP protocol and get the socket file descriptor */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    
    bzero((char *)&serverAddr, sizeof(serverAddr));
    port = atoi(argv[1]);
    
    /* Set the server socketaddr_in members with appropriate values */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    /* Bind the socket with the given server parameters */
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        error("ERROR: Failed to bind socket");

    /* If binding succeeds, put the server socket on listen mode to listen to client requests */
    listen(sockfd, 5);
    clientLen = sizeof(clientAddr);

    // Set up signal handler to manage exiting child processes gracefully and prevent zombies
    signal(SIGCHLD, handle_child_exit);
    
    /* Prepare to accept new clients indefintely. Spawn a new process to process every clients request separately */
    while(1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientLen);
        if (newsockfd < 0)
            error("ERROR on accept");
        pid = fork();
        
        if (pid < 0)
            error("ERROR on fork");

        if (pid == 0){ /* Child process */
            close(sockfd);
            dostuff(newsockfd);
            close(newsockfd);
            exit(0);
        }
    }

    close(sockfd);
    
    return 0; /* We never get here unless the constant in the text segment in memory magically changes to 0 for the while loop */
}

/**
 * @brief There is a separate instance of this function
 * for each connection.  It handles all communication
 * once a connnection has been established.
 * 
 * @param fd The file descriptor to communicate with the client derived from the accept call
 */
void dostuff(int fd)
{
    int numBytes;
    char buffer[256];

    bzero(buffer, 256);
    numBytes = read(fd, buffer, 255);
    if (numBytes < 0)
        error("ERROR: Failed to read from socket");
    
    printf("Here is the message from the client: %s\n", buffer);
    
    const char* response = "Server response: I got your message";
    numBytes = write(fd, response, strlen(response));
    if (numBytes < 0)
        error("ERROR: Failed to write to socket");
}

void handle_child_exit()
{
    int wstat;
    pid_t pid = wait3(&wstat, WNOHANG, NULL);

    if (pid == -1)
        return;
    if (WIFEXITED(wstat)){
        int exit_status = WEXITSTATUS(wstat);
        if (exit_status != 0)
            printf("ERROR: Forked service exited with status: %d\n", exit_status);       
    }
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
