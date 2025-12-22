#ifndef INTERFACE_H
#define INTERFACE_H

#include <arpa/inet.h>
#include <linux/if_packet.h>

struct iface 
{
    size_t ifindex;
    int sock;
    struct sockaddr_ll addr;
    char *ifname;
};

#endif