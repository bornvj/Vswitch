#include <malloc.h>

#include "frame.h"


frame* parseFrame(const unsigned char* buf, size_t size)
{
    if (size <= 14) // src + dst + type size
        return NULL; 

    frame *ret = malloc(sizeof(frame));

    if (!ret)
        return NULL;

    memcpy(ret->dst, buf, 6);
    memcpy(ret->src, buf + 6, 6);   

    ret->type = ntohs(*(uint16_t*)(buf + 12));

    size_t cursor = 14;

    if (ret->type == 0x8100) // VLAN
    {
        ret-> vlan_tci = ntohs(*(uint16_t*)(buf + cursor));
        cursor += 2;
        ret->type = ntohs(*(uint16_t*)(buf + cursor)); // get real type
        cursor += 2;
    }

    ret->payload = buf + cursor;
    ret->payload_size = size - cursor;

    return ret;
}

void printFrame (frame *f)
{
    if (!f)
    {
        printf("NULL FRAME\n");
        return;
    }

    printf("dst [%02x:%02x:%02x:%02x:%02x:%02x] src[%02x:%02x:%02x:%02x:%02x:%02x] \
        type[%02u] payloadSize[%lu]\n", 
        f->dst[0],f->dst[1],f->dst[2],f->dst[3],f->dst[4],f->dst[5],
        f->src[0],f->src[1],f->src[2],f->src[3],f->src[4],f->src[5],
        f->type, f->payload_size);

    // TODO: implements VLAN print
}