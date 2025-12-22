#define _GNU_SOURCE

#include <sys/select.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_link.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "interface.h"
#include "frame.h"
#include "record.h"

#define BUFFERSIZE 1600
#define MAX_IFACES 20

struct iface ifaces[MAX_IFACES];

int main(void)
{
    struct ifaddrs *ifaddr;

    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    size_t nbr_ifaces = 0;

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_PACKET)
            continue;


        printf("%-8s\n", ifa->ifa_name);

        int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sock < 0) 
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_ll addr;
        memset(&addr, 0, sizeof(addr));
        addr.sll_family   = AF_PACKET;
        addr.sll_protocol = htons(ETH_P_ALL);
        addr.sll_ifindex  = ((struct sockaddr_ll *) ifa->ifa_addr)->sll_ifindex;

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
        {
            perror("bind");
            exit(1);
        }
        
        ifaces[nbr_ifaces].ifindex = addr.sll_ifindex;
        ifaces[nbr_ifaces].sock    = sock;
        ifaces[nbr_ifaces].addr    = addr;
        ifaces[nbr_ifaces].ifname  = strdup(ifa->ifa_name);

        nbr_ifaces++;
    }

    freeifaddrs(ifaddr);

    unsigned char buf[BUFFERSIZE];

    while (1)
    {
        fd_set readfds;
        FD_ZERO(&readfds);

        int maxfd = 0;
        for (size_t i = 0; i < nbr_ifaces; i++) 
        {
            FD_SET(ifaces[i].sock, &readfds);
            if (ifaces[i].sock > maxfd)
                maxfd = ifaces[i].sock;
        }

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) 
        {
            perror("select");
            continue;
        }

        for (size_t in = 0; in < nbr_ifaces; in++)
        {
            if (!FD_ISSET(ifaces[in].sock, &readfds))
                continue;

            struct sockaddr_ll src_addr;
            socklen_t addrlen = sizeof(src_addr);

            ssize_t len = recvfrom(ifaces[in].sock, buf, sizeof(buf), 0, (struct sockaddr *)&src_addr, &addrlen);
            if (len <= 0)
                continue;

            frame *f = parseFrame(buf, len);

            if (!f)
                continue;

            mac_table_learn(f->src, f->vlan_id, ifaces[in].ifindex);

            record *dst = mac_table_lookup(f->dst, f->vlan_id);

            if (dst && dst->INTERFACE != ifaces[in].ifindex)
            {
                int out = dst->INTERFACE;

                sendto(ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ifaces[out].addr,
                    sizeof(ifaces[out].addr));
            }
            else
            {
                for (size_t out = 0; out < nbr_ifaces; out++)
                {
                    if (ifaces[out].ifindex == ifaces[in].ifindex)
                        continue;

                    sendto(ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ifaces[out].addr,
                        sizeof(ifaces[out].addr));
                }
            }
            free(f);
        }
        mac_table_age(time(NULL));
    }

    exit(EXIT_SUCCESS);
}