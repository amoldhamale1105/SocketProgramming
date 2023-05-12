#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/version.h>

/* The can_frame struct was changed in commit hash ea78005
   The following command obtains closest tag release after the above commit
   git describe --contains ea78005 => v5.11-rc1 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
   #define PAYLOAD_LEN can_dlc
#else
   #define PAYLOAD_LEN len
#endif

void error(const char*);
void constructDataFrame(const char*,char*,int);

int main(int argc, char** argv)
{
    int sockfd, numBytes;
	struct sockaddr_can senderAddr;
	struct ifreq ifaceReq;
	struct can_frame frame;

    /* Check if required number of args are provided to the program */
    if (argc < 6){
        fprintf(stderr, "usage: %s <can_interface> <frame_format> <can_id_hex> <databyte_frame_hex> <rep_rate_ms>\n", argv[0]);
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

    /* Set the sender socket CAN struct and bind it to the socket descriptor */
    memset(&senderAddr, 0, sizeof(struct sockaddr_can));
    senderAddr.can_family = AF_CAN;
    senderAddr.can_ifindex = ifaceReq.ifr_ifindex;

    if (bind(sockfd, (const struct sockaddr*)&senderAddr, sizeof(senderAddr)) < 0)
        error("ERROR: Failed to bind socket");

    /* Set can ID, frame format and data length based on command line arguments */
    frame.can_id = (canid_t)strtol(argv[3], NULL, 16);
    frame.can_id |= strcasecmp("eff", argv[2]) == 0 ? CAN_EFF_FLAG : 0;
    int frameLen = strlen(argv[4])/2 + strlen(argv[4])%2;
    frame.PAYLOAD_LEN = (unsigned char)(frameLen);
    
    /* Construct data frame from homogenous character string received on the command line */
    char dataFrame[frameLen];
    bzero(dataFrame, sizeof(dataFrame));
    constructDataFrame(argv[4], dataFrame, frameLen);
    bzero(frame.data, 8);
    strcpy(frame.data, dataFrame);

    /* Set CAN frame repetition rate in milliseconds */
    int repRateMS = atoi(argv[5]);

    /* Send the frame after every repetition rate interval */
    while (1)
    {
        numBytes = write(sockfd, &frame, sizeof(frame));
        if (numBytes < 0){
            fprintf(stderr, "Failed to send a frame!");
            continue;
        }
        usleep(1000*repRateMS);
    }
    
    close(sockfd);

    return 0;
}

void constructDataFrame(const char* chunk, char* frame, int count)
{
    int numChars = strlen(chunk);

    /* If the number of chars is odd, create a byte with the odd hex digit in LSB */
    if (numChars%2 != 0){
        char last[2];
        last[0] = chunk[numChars-1];
        last[1] = 0;
        frame[count-1] = (char)strtol(last, NULL, 16);
        count--;
    }

    /* Other hex digits can then be paired to form bytes in the data frame */
    for(int i = 0; i < count; ++i)
    {
        char byte[3];
        byte[0] = chunk[i*2];
        byte[1] = chunk[i*2+1];
        byte[2] = 0;

        frame[i] = (char)strtol(byte, NULL, 16);
    }
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}