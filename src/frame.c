#include <malloc.h>

#include "record.h"
#include "frame.h"
#include "tools.h"


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

    if (ret->type == VLAN) // VLAN
    {
        uint16_t vlan_tci = ntohs(*(uint16_t*)(buf + cursor));
        ret->vlan_priority  = vlan_tci >> 13 & 0b111;
        ret->vlan_cfi       = vlan_tci >> 12 & 0b1;
        ret->vlan_id        = vlan_tci >> 00 & 0b111111111111;

        cursor += 2;
        ret->type = ntohs(*(uint16_t*)(buf + cursor));
        cursor += 2;
    }
    else
    {
        ret->vlan_id = 0;
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
        type[", 
        f->dst[0],f->dst[1],f->dst[2],f->dst[3],f->dst[4],f->dst[5],
        f->src[0],f->src[1],f->src[2],f->src[3],f->src[4],f->src[5]);

    switch (f->type)
    {
        case IPV4:
            printf("IPV4");
            break;
        case IPV6:
            printf("IPV6");
            break;
        case LLDP:
            printf("LLDP");
            break;
        case ARP:
            printf("ARP");
            break;
        case HOMEPLUG2:
        case HOMEPLUG :
            printf("HOMEPLUG");
            break;
        default:
            printf("0x%04x", f->type);
            break;
    }

    printf("] payloadSize[%lu]\n",f->payload_size);
    // TODO: implements VLAN print
}

void handleFrame(frame *f, switch_ctx *ctx, size_t in, ssize_t len, unsigned char *buf)
{
    ctx->ifaces[in].rx_frames += 1;
    ctx->ifaces[in].rx_bytes += len;

    // learn
    mac_table_learn(ctx->mac_table, f->src, f->vlan_id, ctx->ifaces[in].ifindex);

    // if the dest is the receiver iface
    if (mac_equal(f->dst, ctx->ifaces[in].mac))
        return;

    // lookup for destination
    record *dst = mac_table_lookup(ctx->mac_table, f->dst, f->vlan_id);

    if (dst && dst->INTERFACE != ctx->ifaces[in].ifindex)
    {
        int out = dst->INTERFACE;

        sendto(ctx->ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ctx->ifaces[out].addr,
            sizeof(ctx->ifaces[out].addr));

        ctx->ifaces[out].rx_frames += 1;
        ctx->ifaces[out].rx_bytes += len;
    }
    else
    {
        for (size_t out = 0; out < ctx->nbr_ifaces; out++)
        {
            if (ctx->ifaces[out].ifindex == ctx->ifaces[in].ifindex)
                continue;

            sendto(ctx->ifaces[out].sock, buf, len, 0, (struct sockaddr *)&ctx->ifaces[out].addr,
                sizeof(ctx->ifaces[out].addr));

            ctx->ifaces[out].tx_frames += 1;
            ctx->ifaces[out].tx_bytes += len;
        }
    }
}