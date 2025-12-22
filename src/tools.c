#include <stdio.h>

#include "tools.h"
#include "record.h"

uint8_t hash(const unsigned char MAC[6], uint16_t vlan)
{
    uint16_t ret = MAC[5] ^ MAC[4] ^ MAC[3] ^
            MAC[2] ^ MAC[1] ^ MAC[0] ^ vlan;

    return ret % UINT8_MAX;
}

int mac_equal(const unsigned char MAC1[6], const unsigned char MAC2[6])
{
    return !memcmp(MAC1, MAC2, 6);
}

void print_mac_table(struct iface *ifaces, size_t iface_nbr, time_t start_time)
{
    printf("\033[H\033[J");
    bucket *b;

    time_t now = time(NULL);

    printf("uptime: %ldd %ldh %ldm %lds\n",
       (long)((now - start_time) / 86400),
       (long)(((now - start_time) % 86400) / 3600),
       (long)(((now - start_time) % 3600) / 60),
       (long)((now - start_time) % 60));

    for (size_t iface = 0; iface < iface_nbr; iface++)
    {
        printf("\n╔═════════════════════════════════════════════════════════╗\n");
        printf("║%2lu | \033[31m%-8s\033[0m rcv: %4lu",
             ifaces[iface].ifindex, ifaces[iface].ifname, 
             ifaces[iface].rx_frames); 


        if (ifaces[iface].rx_bytes > 1000000)
            printf("(%6lu %s)", ifaces[iface].rx_bytes / 1000000, "Mb");
        else if ((ifaces[iface].rx_bytes > 1000))
            printf("(%6lu %s)", ifaces[iface].rx_bytes / 1000, "Kb");
        else 
            printf("(%6lu %s)", ifaces[iface].rx_bytes, " b");


        printf(" | snd: %4lu",ifaces[iface].tx_frames);

        if (ifaces[iface].tx_bytes > 1000000)
            printf("(%6lu %s)", ifaces[iface].tx_bytes / 1000000, "Mb");
        else if ((ifaces[iface].tx_bytes > 1000))
            printf("(%6lu %s)", ifaces[iface].tx_bytes / 1000, "Kb");
        else 
            printf("(%6lu %s)", ifaces[iface].tx_bytes, " b");

        printf("║\n╚═════════════════════════════════════════════════════════╝\n");
        for (size_t index = 0; index < BUCKETS_SIZE; index++)
        {
            b = mac_table[index];
            while (b)
            {
                if (b->rec->INTERFACE == iface)
                {
                    printf("   [%02x:%02x:%02x:%02x:%02x:%02x] - VLAN[%4d] - last_seen: %lu\n",
                        b->rec->MAC[0],b->rec->MAC[1],b->rec->MAC[2],
                        b->rec->MAC[3],b->rec->MAC[4],b->rec->MAC[5],
                        b->rec->VLAN,
                        now - b->rec->last_seen);
                }
                b = b->next;
            }
        }
    }
}