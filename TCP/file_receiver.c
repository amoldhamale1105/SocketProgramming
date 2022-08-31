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
    
    /* Connect to the file sender */
    if (connect(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) 
        error("ERROR: Failed to connect to the server");
    printf("Connected to file sender %d:%d\n", serverAddr.sin_addr.s_addr, serverAddr.sin_port);

    printf("\nEnter absolute local directory path to save all files from the sender:\n(Make sure the directory has write permissions for %s)\n", getenv("USER"));
    fgets(buffer, 255, stdin);
    
    /* Get the exact path length and filename length and neglect the newline character introduced by fgets */
    int path_len = 0, filename_len = 0, size_len = 0;
    while (*(buffer+path_len) != '\n')
    {
        path_len++;
    }

    /* Add a leading forward slash for directory in case the user does not mention it */
    if (buffer[path_len-1] != '/'){
        buffer[path_len] = '/';
        path_len++;
    }
    else
        buffer[path_len] = 0;
    
    char filepath[path_len+100];
    bzero(filepath, sizeof(filepath));
    strcpy(filepath, buffer);

    /* Send a signal to the sender indicating that the client is ready to receive files from it */
    numBytes = write(sockfd, "r", 1);
    if (numBytes < 0)
        error("ERROR: Failed to write to socket");
    
    /* Receive multiple files from the sender until the connection exists */
    while (1)
    {
        printf("\nWaiting for the server to send a file...\n(Ctrl+C to stop receiving files from server)\n");
        
        /* First get the file meta data like size and name */
        bzero(buffer, sizeof(buffer));
        numBytes = recv(sockfd, buffer, 255, 0);
        if (numBytes < 0)
            error("ERROR: Failed to read from socket");
        ssize_t file_size = atol(buffer);
        while (*(buffer+size_len) != 0)
        {
            size_len++;
        }
        strcpy(filepath+path_len, buffer+size_len+1);

        /* Try opening/creating the file and send an ack accordingly */
        int file_fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file_fd < 0)
            error("Failed to open or create file");
        
        numBytes = write(sockfd, "y", 1);
        if (numBytes < 0)
            error("ERROR: Failed to write to socket");
        
        printf("Downloading file: %s\n", filepath+path_len);
        size_t bytes_received = receive_file(file_fd, sockfd, 0, file_size);
        fprintf(stdout, "Recevied %s of size %ld bytes\n", filepath+path_len, bytes_received);
        close(file_fd);
    }
    
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
