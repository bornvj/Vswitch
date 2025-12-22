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

void print_mac_table(struct iface *ifaces, size_t iface_nbr)
{
    printf("\033[H\033[J");
    bucket *b;

    time_t now = time(NULL);

    for (size_t iface = 0; iface < iface_nbr; iface++)
    {
        printf("%2lu | %-8s \n", ifaces[iface].ifindex, ifaces[iface].ifname);
        printf("==============================\n");
        for (size_t index = 0; index < BUCKETS_SIZE; index++)
        {
            b = mac_table[index];
            while (b)
            {
                if (b->rec->INTERFACE == iface)
                {
                    printf("-> [%02x:%02x:%02x:%02x:%02x:%02x] - VLAN[%4d] - last_seen: %lu\n",
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