#include <stdio.h>
#include <string.h>

#include "command.h"

command *parseCommand(unsigned char *buf, size_t len)
{
    if (!buf || !len)
        return NULL;

    char cmd[32] = {0}, arg1[32] = {0}, arg2[32] = {0};

    if (sscanf((char *)buf, "%31s %31s %31s", cmd, arg1, arg2) < 0)
        return NULL;

    command *ret = malloc(sizeof(command));
    ret->type = ERROR;
    ret->ifname[0] = '\0';

    if (strcmp(cmd, "GET") == 0)
    {
        if (strcmp(arg1, "IFACES") == 0)
        {
            ret->type = GET_IFACES;
        }
        else if (strcmp(arg1, "DATA") == 0)
        {
            ret->type = GET_DATA;
        } else
        {
            free(ret);
            return NULL;
        }
    }
    else if (strcmp(cmd, "SET") == 0)
    {

    } else
    {

    }
    
    return ret;
}

void printCommand(command *cmd)
{
    if (!cmd)
    {
        printf("NULL COMMAND\n");
        return;
    }

    switch (cmd->type)
    {
        case GET_IFACES:
            printf("GET IFACES\n");
            break;
        case GET_DATA:
            printf("GET DATA\n");
            break;
        default:
            printf("ERROR COMAND\n");
            break;
    }
}

void handleCommand(command *cmd, char* outputBuf, switch_ctx ctx, time_t now)
{
    if (!cmd)
    {
        strcpy(outputBuf, "ERROR");
        return;
    }

    switch (cmd->type)
    {
        case GET_IFACES:
        {
            outputBuf[0] = '\0';
            strcat(outputBuf, "{\"ifaces\" : [");

            if (ctx.nbr_ifaces >= 1)
            {
                strcat(outputBuf, "\"");
                strcat(outputBuf, ctx.ifaces[0].ifname);
                strcat(outputBuf, "\"");
            }

            for (size_t i = 1; i < ctx.nbr_ifaces; i++)
            {
                strcat(outputBuf, ",\"");
                strcat(outputBuf, ctx.ifaces[i].ifname);
                strcat(outputBuf, "\"");
            }

            strcat(outputBuf, "]}");
            break;
        }
        case GET_DATA:
            outputBuf[0] = '\0';
            char tem_string[1024] = {'\0'};
            strcat(outputBuf, "{"); // start json
            {
                strcat(outputBuf, "\"ifaces\" : ["); // start ifaces
                for (size_t i = 0; i < ctx.nbr_ifaces; i++)
                {
                    tem_string[0] = '\0';
                    sprintf(tem_string, 
                        "{\
                            \"name\": \"%s\", \
                            \"rx_frames\": \"%lu\", \
                            \"rx_bytes\": \"%lu\",  \
                            \"tx_frames\": \"%lu\", \
                            \"tx_bytes\": \"%lu\",  \
                            \"mac \" : [", 
                        ctx.ifaces[i].ifname,
                        ctx.ifaces[i].rx_frames,
                        ctx.ifaces[i].rx_bytes,
                        ctx.ifaces[i].tx_frames,
                        ctx.ifaces[i].tx_bytes);
                    strcat(outputBuf, tem_string);
                    
                    int first_mac = 1;
                    for (size_t i = 0; i < sizeof(ctx.mac_table) / sizeof(bucket*); i++)
                    {
                        bucket *cur = ctx.mac_table[i];
                        while (cur)
                        {
                            if (cur->rec->INTERFACE == i)
                            {
                                if (first_mac)
                                    first_mac = 0;
                                else
                                    strcat(outputBuf, ",");
                                tem_string[0] = '\0';
                                sprintf(tem_string, "{\"address\" : %02x:%02x:%02x:%02x:%02x:%02x, \"last_seen\" : %lu}",
                                cur->rec->MAC[0],cur->rec->MAC[1],cur->rec->MAC[2],cur->rec->MAC[3],cur->rec->MAC[4],cur->rec->MAC[5], now - cur->rec->last_seen);
                                strcat(outputBuf, tem_string);
                            }
                            cur = cur->next;
                        }
                    }

                    strcat(outputBuf, "]}");
                    if (i != ctx.nbr_ifaces - 1)
                        strcat(outputBuf, ",");
                }
                strcat(outputBuf, "]"); // end ifaces
            strcat(outputBuf, "}"); // start json
            }
            break;
        default:
            strcpy(outputBuf, "ERROR");
            break;
    }
}