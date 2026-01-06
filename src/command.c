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

    printf("cmd=%s, arg1=%s, arg2=%s\n", cmd, arg1, arg2);
    command *ret = malloc(sizeof(command));
    ret->type = ERROR;
    ret->ifname[0] = '\0';

    if (strcmp(cmd, "GET") == 0)
    {
        if (strcmp(arg1, "IFACES") == 0)
        {
            ret->type = GET_IFACES;
        }
        else if (strcmp(arg1, "TRAFIC") == 0)
        {
            ret->type = GET_TRAFIC;
            strcpy(ret->ifname, arg2);
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
        case GET_TRAFIC:
            printf("GET TRAFIC %s\n", cmd->ifname);
            break;
        default:
            break;
    }
}

void handleCommand(command *cmd, char* outputBuf, switch_ctx ctx)
{
    if (!cmd)
    {
        strcpy(outputBuf, "ERROR");
        return;
    }

    switch (cmd->type)
    {
        case GET_IFACES:
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
        case GET_TRAFIC:
            for (size_t i = 0; i < ctx.nbr_ifaces; i++)
            {
                if (strcmp(ctx.ifaces[i].ifname, cmd->ifname) == 0)
                {   
                    outputBuf[0] = '\0';
                    sprintf(outputBuf, 
                        "{\
                            \"rx_frames\": \"%lu\", \
                            \"rx_bytes\": \"%lu\", \
                            \"tx_frames\": \"%lu\", \
                            \"tx_bytes\": \"%lu\" \
                        }", 
                        ctx.ifaces[i].rx_frames,
                        ctx.ifaces[i].rx_bytes,
                        ctx.ifaces[i].tx_frames,
                        ctx.ifaces[i].tx_bytes);
                }
            }
            break;
        default:
            strcpy(outputBuf, "ERROR");
            break;
    }
}