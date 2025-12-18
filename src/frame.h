#ifndef FRAME_H
#define FRAME_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

typedef struct frame {
    unsigned char dst[6];
    unsigned char src[6];
    uint16_t type;
    uint16_t vlan_tci;
    const unsigned char* payload;
    size_t payload_size;
} frame;

frame *parseFrame(const unsigned char* buf, size_t size);

void printFrame(frame *f);

#endif