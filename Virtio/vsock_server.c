#include <sys/socket.h>
#include <linux/vm_sockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void error(const char *);

int main(int argc, char **argv)
{
	struct sockaddr_vm addr, peer_addr;
	char buf[256];

	/* Check if required number of args are provided to the program */
	if (argc < 2){
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	/* Create a virtio socket */
	int sockfd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR: Failed to open socket");

	memset(&addr, 0, sizeof(struct sockaddr_vm));
	addr.svm_family = AF_VSOCK;
	addr.svm_port = atoi(argv[1]);
	addr.svm_cid = VMADDR_CID_ANY; /* Bind to any CID to facilitate client connection from host or nested VM */

	if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_vm)) < 0)
		error("ERROR: Failed to bind with socket");

	listen(sockfd, 5);

	socklen_t peer_addr_size = sizeof(struct sockaddr_vm);
	int peer_fd = accept(sockfd, (struct sockaddr *)&peer_addr, &peer_addr_size);
	if (peer_fd < 0)
		error("ERROR: Failed to establish connection with client");

	/* Send and receive to a connected virtio client */
	bzero(buf, 256);
	int numBytes = recv(peer_fd, buf, 255, 0);
	if (numBytes < 0)
		error("ERROR: Failed to receive from client");

	printf("Message from the client: %s\n", buf);

	const char *response = "Server response: I got your message";
	numBytes = send(peer_fd, response, strlen(response), 0);
	if (numBytes < 0)
		error("ERROR: Failed to send to client");

	/* Clean up */
	close(peer_fd);
	close(sockfd);

	return 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}
