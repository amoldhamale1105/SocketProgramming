#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

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

    sockfd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 1)
        error("ERROR: Failed to create socket");
    
    bzero(&ifaceReq, sizeof(struct ifreq));
    strcpy(ifaceReq.ifr_name, argv[1]);
    ioctl(sockfd, SIOCGIFINDEX, &ifaceReq);

    memset(&senderAddr, 0, sizeof(struct sockaddr_can));
    senderAddr.can_family = AF_CAN;
    senderAddr.can_ifindex = ifaceReq.ifr_ifindex;

    if (bind(sockfd, (const struct sockaddr*)&senderAddr, sizeof(senderAddr)) < 0)
        error("ERROR: Failed to bind socket");

    frame.can_id = (canid_t)strtol(argv[3], NULL, 16);
    frame.can_id |= strcasecmp("eff", argv[2]) == 0 ? CAN_EFF_FLAG : 0;
    int frameLen = strlen(argv[4])/2 + strlen(argv[4])%2;
    frame.len = (unsigned char)(frameLen);
    
    char dataFrame[frameLen];
    bzero(dataFrame, sizeof(dataFrame));
    constructDataFrame(argv[4], dataFrame, frameLen);
    bzero(frame.data, 8);
    strcpy(frame.data, dataFrame);

    int repRateMS = atoi(argv[5]);

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

    if (numChars%2 != 0){
        char last[2];
        last[0] = chunk[numChars-1];
        last[1] = 0;
        frame[count-1] = (char)strtol(last, NULL, 16);
        count--;
    }

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