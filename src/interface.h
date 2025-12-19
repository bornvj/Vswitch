#ifndef INTERFACE_H
#define INTERFACE_H

#include <arpa/inet.h>

struct iface 
{
    int ifindex;
    int sock;
    struct sockaddr_ll addr;
};

#endif