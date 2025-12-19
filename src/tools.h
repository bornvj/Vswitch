#ifndef TOOLS_H
#define TOOLS_H

#include <stdint.h>
#include <unistd.h>
#include <string.h>

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

#endif