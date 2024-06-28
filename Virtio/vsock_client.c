#include <sys/socket.h>
#include <stdio.h>
#include <linux/vm_sockets.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void error(const char *);

int main(int argc, char** argv)
{
	struct sockaddr_vm addr;
	char buf[256];

	/* Check if required number of args are provided to the program */
    if (argc < 2){
        fprintf(stderr, "usage: %s <port> [cid]\n", argv[0]);
        exit(1);
    }
	/* Create a virtio socket */
	int sockfd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR: Failed to open socket");

	memset(&addr, 0, sizeof(struct sockaddr_vm));
	addr.svm_family = AF_VSOCK;
	addr.svm_port = atoi(argv[1]);
	addr.svm_cid = argc > 2 ? atoi(argv[2]) : VMADDR_CID_HOST; /* Connect to VM with specified context ID or host */

	if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_vm)) < 0)
		error("ERROR: Failed to connect to the virtio server");

	const char* msg = "Hey there! I'm using virtio";
	int numBytes = send(sockfd, msg, strlen(msg), 0);
	if (numBytes < 0)
         error("ERROR: Failed to write to socket");
	
	/* Clear the buffer and prepare it to receive a response from the server */
    bzero(buf, 256);
    numBytes = recv(sockfd, buf, 255, 0);
    if (numBytes < 0) 
        error("ERROR: Failed to read from socket");
    printf("%s\n", buf);
    
	/* Clean up */
    close(sockfd);

	return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

