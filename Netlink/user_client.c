#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define KERNEL_PORT 0
#define NETLINK_USER 30
#define PAYLOAD_SIZE 1024

struct msghdr msg;

void error(const char *);

int main(void)
{
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    int sock_fd;
    
    /* Create a netlink socket protocol familty set to one of available 32 units */
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
        error("ERROR: Failed to create socket");

    /* Set the socket structure with family and port number details. By default port number is the process PID */
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    /* Bind the socket with the given server parameters */
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    /* Prepare the destination socket structure */
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    /* In netlink protocol, the kernel port is always 0 */
    dest_addr.nl_pid = KERNEL_PORT;
    /* Group ID 0 implies unicast messaging */
    dest_addr.nl_groups = 0;

    /* Allocate space for aligned netlink header + payload */
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(PAYLOAD_SIZE));
    memset(nlh, 0, NLMSG_SPACE(PAYLOAD_SIZE));
    /* Total length of message will be header length + payload size */
    nlh->nlmsg_len = NLMSG_SPACE(PAYLOAD_SIZE);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    strcpy(NLMSG_DATA(nlh), "Hello from userspace process");

    /* Set the base address and length in bytes of the data with the iovec data structure */
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    /* Prepare the message header to be sent with the sendmsg function */
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    /* Here length implies number of elements in the iovec, which is 1 */
    msg.msg_iovlen = 1;

    printf("Sending message to kernel (PID %d)\n", src_addr.nl_pid);
    sendmsg(sock_fd, &msg, 0);
    printf("Waiting to hear from kernel\n");

    /* Receive message from kernel */
    recvmsg(sock_fd, &msg, 0);
    printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));
    
    close(sock_fd);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}