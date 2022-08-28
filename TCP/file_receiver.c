#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void error(const char*);
ssize_t receive_file(int,int,off_t,size_t);

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
    
    /* Once connection succeeds, send the filename to be downloaded to the server */
    printf("Enter the filename with absolute path to be fetched from the server:\n");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    /* Get the exact path length and filename length and neglect the newline character introduced by fgets */
    int path_len = 0, filename_len = 0;
    while (*(buffer+path_len) != '\n')
    {
        path_len++;
    }
    while (filename_len < path_len && *(buffer+path_len-filename_len) != '/')
    {
        filename_len++;
    }

    char filename[filename_len];
    bzero(filename, filename_len);
    if (filename_len <= 0)
        error("ERROR: Invalid path or filename");
    strncpy(filename, buffer+path_len-filename_len+1, filename_len-1);

    /* Send full path to the server for the file to be downloaded */
    numBytes = write(sockfd, buffer, path_len);
    if (numBytes < 0)
         error("ERROR: Failed to write to socket");
    
    /* First get the file size and then send an ack to start downloading */
    bzero(buffer, sizeof(buffer));
    numBytes = recv(sockfd, buffer, 255, 0);
    if (numBytes < 0)
         error("ERROR: Failed to read from socket");
    ssize_t file_size = atol(buffer);

    /* Here we can check if file size does not exceed our limit and send an ack accordingly */
    numBytes = write(sockfd, "y", 1);
    if (numBytes < 0)
         error("ERROR: Failed to write to socket");

    int file_fd = open(filename, O_WRONLY | O_CREAT, 0644);
    if (file_fd < 0)
        error("Failed to open or create file");
    
    printf("Downloading file: %s\n", filename);
    size_t bytes_received = receive_file(file_fd, sockfd, 0, file_size);
    fprintf(stdout, "Recevied file %s of size %ld\n", filename, bytes_received);
    
    close(file_fd);
    close(sockfd);
    
    return 0;
}

ssize_t receive_file(int out_fd, int in_fd, off_t offset, size_t count)
{
    int pipefd[2];
    ssize_t bytes, bytes_received, bytes_in_pipe;
    ssize_t remaining_data = count;

    if (pipe(pipefd) < 0)
        error("Failed to create pipe");
    
    /* Zero-copy transfer of data from socket to the write end of the pipe
       followed by read end of pipe to the local file descriptor */
    while (remaining_data > 0)
    {
        if ((bytes_received = splice(in_fd, NULL, pipefd[1], NULL, remaining_data, SPLICE_F_MOVE | SPLICE_F_MORE)) <= 0)
            error("Splicing to pipe failed");

        bytes_in_pipe = bytes_received;

        /* Move data stored in the pipe to our desired file */
        while (bytes_in_pipe > 0)
        {
            if ((bytes = splice(pipefd[0], NULL, out_fd, &offset, bytes_in_pipe, SPLICE_F_MOVE | SPLICE_F_MORE)) <= 0)
                error("Splicing from pipe failed");
            bytes_in_pipe -= bytes;
        }

        remaining_data -= bytes_received;
    }

    return count - remaining_data;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
