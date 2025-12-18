#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <string.h>

#include "frame.h"
#include "record.h"

#define BUFFERSIZE 1600
#define INGRESS_IF 2

int main(int argc, char *argv[]) 
{

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family =       AF_PACKET;
    addr.sll_protocol =     htons(ETH_P_ALL);
    addr.sll_ifindex =      INGRESS_IF;
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    unsigned char buf[BUFFERSIZE];
    while (1) 
    {
        socklen_t addrlen = sizeof(struct sockaddr_ll);
        ssize_t len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addrlen);
        if (len < 0)
            continue;

        frame *f = parseFrame(buf, len);
        printFrame(f);

        mac_table_learn(f->src, f->vlan_id, INGRESS_IF);

        record *rec = mac_table_lookup(f->dst, f->vlan_id);
        
        if (rec)
        {
            // deposer le colis
        }
        else
        {
            //flood le colis
        }
    
        free(f);
    }
    return 0;
}
