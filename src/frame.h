#ifndef FRAME_H
#define FRAME_H

/**
 * @file frame.h
 * @brief define a frame
 */


#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

#include "switch_ctx.h"

enum EtherType
{
    IPV4        = 0x0800,
    IPV6        = 0x86DD,
    LLDP        = 0x88CC,
    ARP         = 0x0806,
    HOMEPLUG    = 0x88E1,
    HOMEPLUG2   = 0x887b,

    VLAN    = 0x8100, // not a type
};


/**
 * @brief Contains information parsed from a IEEE ethernet frame
 */
typedef struct frame 
{
    unsigned char dst[6];
    unsigned char src[6];
    uint16_t type;
    uint8_t vlan_priority;
    uint8_t vlan_cfi;
    uint16_t vlan_id;
    const unsigned char* payload;
    size_t payload_size;
} frame;

frame *parseFrame(const unsigned char* buf, size_t size);

void printFrame(frame *f);

void handleFrame(frame *f, switch_ctx *ctx, size_t in, ssize_t len, unsigned char *buf);

#endif