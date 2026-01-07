#ifndef INTERFACE_H
#define INTERFACE_H

/**
 * @file interface.h
 * @brief interface struct definition
 */


#include <arpa/inet.h>
#include <linux/if_packet.h>

struct iface 
{
    size_t ifindex;
    int sock;
    struct sockaddr_ll addr;
    char *ifname;
    unsigned char mac[6];

    unsigned long rx_frames;    // received frames
    unsigned long rx_bytes;     // recieved bytes
    unsigned long tx_frames;    // sent frame
    unsigned long tx_bytes;     // sent bytes
};

#endif