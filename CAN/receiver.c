#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/wait.h>

#include <linux/can.h>
#include <linux/can/raw.h>

void error(const char*);

int main(int argc, char** argv)
{
    int sockfd, numBytes;
	struct sockaddr_can receiverAddr;
	struct ifreq ifaceReq;
	struct can_frame frame;

    /* Check if required number of args are provided to the program */
    if (argc < 2){
        fprintf(stderr, "usage: %s <can_interface>\n", argv[0]);
        exit(1);
    }

    /* Create a raw CAN socket with Socket CAN protocol */
    sockfd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 1)
        error("ERROR: Failed to create socket");

    /* Get the CAN interface index based on interface name */
    bzero(&ifaceReq, sizeof(struct ifreq));
    strcpy(ifaceReq.ifr_name, argv[1]);
    ioctl(sockfd, SIOCGIFINDEX, &ifaceReq);

    /* Set the receiver socket CAN struct and bind it to the socket descriptor */
    memset(&receiverAddr, 0, sizeof(struct sockaddr_can));
    receiverAddr.can_family = AF_CAN;
    receiverAddr.can_ifindex = ifaceReq.ifr_ifindex;

    if (bind(sockfd, (const struct sockaddr*)&receiverAddr, sizeof(receiverAddr)) < 0)
        error("ERROR: Failed to bind socket");

    printf("Data received from CAN bus %s:\n(Ctrl+C or SIGINT signal to stop reception)\n", argv[1]);
    
    /* Receive all CAN traffic on the specified CAN interface and dump it on the screen */
    while (1)
    {        
        numBytes = read(sockfd, &frame, sizeof(frame));
        if (numBytes < 0){
            fprintf(stderr, "Frame dropped!");
            continue;
        }
        
        /* These special flags on MSB of the frame can be ignored while outputting the ID */
        frame.can_id &= ~(CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_ERR_FLAG);
        printf("ID: 0x%08X Frame size: %d Data bytes: ", frame.can_id, frame.len);
        for (size_t i = 0; i < frame.len; i++)
        {
            printf("0x%02X ", frame.data[i]);
        }
        printf("\r\n");
    }

    close(sockfd);
    
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}