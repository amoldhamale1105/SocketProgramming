/**
 * @file file_sender.c
 * @author Amol Dhamale
 * @brief A general purpose monogamous file sender to copy/transfer any type of file on a network to a single client at a time.
 * In this model, the client once connected can accept files from the sender in a configured location without having to request
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
ssize_t transferFile(int,int,off_t,size_t);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, port, pid, numBytes;
    socklen_t clientLen;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[256];


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

    printf("Waiting for a file receiver client...\n");
    newsockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientLen);
    if (newsockfd < 0)
        error("ERROR on accept");
    printf("Client connected.\nWaiting for client to be ready for download.\n");

    /* Block until the connected client sets target directory for downloading content */
    bzero(buffer, 256);
    numBytes = recv(newsockfd, buffer, 255, 0);
    if (numBytes < 0)
        error("ERROR: Failed to read from socket");
    if (buffer[0] != 'r'){
        printf("Client refused to setup download or exited prematurely\n");
        close(sockfd);
        exit(0);
    }
    printf("Client ready to receive files.\n");
    
    /* Begin the file transfer session with the connected client and continue as long as the client accepts and connection exists */
    while(1)
    {
        bzero(buffer, sizeof(buffer));
        printf("\nEnter filename with absolute path to be sent to the client:\n");
        fgets(buffer, 255, stdin);

        /* Ignore the newline character and compute path length and filename length */
        int path_len = 0, filename_len = 0;
        while (*(buffer+path_len) != '\n')
        {
            path_len++;
        }
        while (filename_len < path_len && *(buffer+path_len-filename_len) != '/')
        {
            filename_len++;
        }
        buffer[path_len] = 0;
        
        /* Extract the filename to be passed to the client */
        char filename[filename_len];
        bzero(filename, filename_len);
        strcpy(filename, buffer+path_len-filename_len+1);

        int file_fd = open(buffer, O_RDONLY);
        if (file_fd < 0)
            error("Failed to open file");
        
        /* Calculate the file size */
        struct stat file_stat;
        if (fstat(file_fd, &file_stat) < 0)
            error("Failed to determine file size");
        ssize_t file_size = file_stat.st_size;
        fprintf(stdout, "File size: %ld\n", file_size);
        
        /* Send file size and name to client to confirm if it wants to download */
        bzero(buffer, sizeof(buffer));
        int countLen = sprintf(buffer, "%ld", file_size);
        strncpy(buffer+countLen+1, filename, filename_len);
        numBytes = send(newsockfd, buffer, countLen+filename_len+1, 0);
        if (numBytes < 0)
            error("ERROR: Failed to write to socket");

        /* Receive ack from client and decide whether to send the file or exit based on it */
        bzero(buffer, 256);
        numBytes = recv(newsockfd, buffer, 255, 0);
        if (numBytes < 0)
            error("ERROR: Failed to read from socket");
        if (buffer[0] != 'y'){
            printf("Canceling file transfer. No positive ack from client\n");
            close(newsockfd);
            exit(0);
        }
        
        ssize_t bytes_sent = transferFile(newsockfd, file_fd, 0, file_size);
        close(file_fd);
        fprintf(stdout, "Sent %s of size %ld bytes\n", filename, bytes_sent);
    }

    close(newsockfd);
    close(sockfd);
    
    return 0;
}

ssize_t transferFile(int out_fd, int in_fd, off_t offset, size_t count)
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

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
