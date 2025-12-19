#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include "interface.h"
#include "frame.h"
#include "record.h"

#define BUFFERSIZE 1600
#define MAX_IFACES 3

struct iface ifaces[MAX_IFACES];

int main(void)
{
    for (int i = 0; i < MAX_IFACES; i++)
    {
        int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sock < 0) 
        {
            perror("socket");
            exit(1);
        }

        struct sockaddr_ll addr;
        memset(&addr, 0, sizeof(addr));
        addr.sll_family   = AF_PACKET;
        addr.sll_protocol = htons(ETH_P_ALL);
        addr.sll_ifindex  = i + 1;   // TODO : FIND REEL

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
        {
            perror("bind");
            exit(1);
        }

        ifaces[i].ifindex = addr.sll_ifindex;
        ifaces[i].sock    = sock;
        ifaces[i].addr    = addr;
    }

    unsigned char buf[BUFFERSIZE];

    while (1)
    {
        fd_set readfds;
        FD_ZERO(&readfds);

        int maxfd = 0;
        for (int i = 0; i < MAX_IFACES; i++) 
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

        for (int in = 0; in < MAX_IFACES; in++)
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

            mac_table_learn(f->src, f->vlan_id, in);

            record *dst = mac_table_lookup(f->dst, f->vlan_id);

            if (dst && dst->INTERFACE != in)
            {
                int out = dst->INTERFACE;

                sendto(ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ifaces[out].addr,
                    sizeof(ifaces[out].addr));
            }
            else
            {
                for (int out = 0; out < MAX_IFACES; out++)
                {
                    if (out == in)
                        continue;

                    sendto(ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ifaces[out].addr,
                        sizeof(ifaces[out].addr));
                }
            }
            free(f);
        }
        mac_table_age(time(NULL));
    }

    return 0;
}