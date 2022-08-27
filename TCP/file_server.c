/**
 * @file file_server.c
 * @author Amol Dhamale
 * @brief A general purpose file server to copy/transfer any type of file on a network, capable of handling multiple clients at the same time
 * @version 0.1
 * @date 2022-08-27
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

void do_send(int);
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

    /* Set up signal handler to manage exiting child processes gracefully and prevent zombies */
    signal(SIGCHLD, handle_child_exit);
    
    /* Run the file server indefintely. Spawn a new process to process every client request separately */
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
            do_send(newsockfd);
            close(newsockfd);
            exit(0);
        }
    }

    close(sockfd);
    
    return 0; /* We never get here unless the constant in the text segment in memory magically changes to 0 for the while loop */
}

static ssize_t transferFile(int out_fd, int in_fd, off_t offset, size_t count)
{
    ssize_t bytes_sent = 0;
    ssize_t remaining_data = count;

    /* Using the sendfile system call to facilitate zero-copy file transfer
       However, sendfile has its own limitations which might warrant usage of splice for sending
       https://yarchive.net/comp/linux/splice.html */
    while (remaining_data > 0)
    {
        if ((bytes_sent = sendfile(out_fd, in_fd, &offset, remaining_data)) <= 0)
            error("Failed to send file");
        remaining_data -= bytes_sent;
    }

    return count - remaining_data;
}

void do_send(int fd)
{
    int numBytes;
    char buffer[256];

    /* Get filename with absolute path from client */
    bzero(buffer, 256);
    numBytes = read(fd, buffer, 255);
    if (numBytes < 0)
        error("ERROR: Failed to read from socket");
    
    printf("\nRequested file: %s\n", buffer);

    int file_fd = open(buffer, O_RDONLY);
    if (file_fd < 0)
        error("Failed to open file");
    
    /* On reception of a valid file, first compute its size */
    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0)
        error("Failed to determine file size");
    ssize_t file_size = file_stat.st_size;
    fprintf(stdout, "File size: %ld\n", file_size);
    
    /* Send file size to client to confirm if it wants to download */
    bzero(buffer, sizeof(buffer));
    int countLen = sprintf(buffer, "%ld", file_size);
    numBytes = send(fd, buffer, countLen, 0);
    if (numBytes < 0)
         error("ERROR: Failed to write to socket");

    /* Receive ack from client and decide whether to send the file or exit based on it */
    bzero(buffer, 256);
    numBytes = recv(fd, buffer, 255, 0);
    if (numBytes < 0)
        error("ERROR: Failed to read from socket");
    if (buffer[0] != 'y'){
        printf("Canceling transfer on account of negative ack from client\n");
        close(fd);
        exit(0);
    }
    
    ssize_t bytes_sent = transferFile(fd, file_fd, 0, file_size);
    close(file_fd);
    fprintf(stdout, "Total data transmitted: %ld bytes\n", bytes_sent);
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
