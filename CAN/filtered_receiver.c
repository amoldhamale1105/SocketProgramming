#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

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
    if (argc < 4){
        fprintf(stderr, "usage: %s <can_interface> <can_id_filter_hex> <frame_format>\n", argv[0]);
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

    /* Set up a filter for a particular ID on the bus with the help of relevant mask
       The bitmask scans only required bits in the ID based on type of frame, extended or standard */
    struct can_filter filter;
    filter.can_id = (canid_t)strtol(argv[2], NULL, 16);
    filter.can_mask = strcasecmp("eff", argv[3]) == 0 ? CAN_EFF_MASK : CAN_SFF_MASK;

    setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));

    printf("Filtered data received from CAN bus %s:\n", argv[1]);
    
    /* Receive filtered CAN traffic for only the ID of our choice whenever it is transmitted by some node on the bus */
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