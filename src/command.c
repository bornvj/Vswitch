#include <stdio.h>
#include <string.h>

#include "command.h"

command *parseCommand(unsigned char *buf, size_t len)
{
    if (!buf || !len)
        return NULL;

    char cmd[32], arg1[32], arg2[32];

    if (sscanf((char *)buf, "%31s %31s %31s", cmd, arg1, arg2) < 0)
        return NULL;

    printf("cmd=%s, arg1=%s, arg2=%s\n", cmd, arg1, arg2);
    command *ret = malloc(sizeof(command));
    ret->type = ERROR;

    if (strcmp(cmd, "GET") == 0)
    {
        if (strcmp(arg1, "IFACES") == 0)
            ret->type = GET_IFACES;
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
            printf("GET IFACES COMMAND\n");
            break;
        
        default:
            break;
    }
}

void handleCommand(command *cmd, char* outputBuf)
{
    if (!cmd)
    {
        strcpy(outputBuf, "ERROR");
        return;
    }

    switch (cmd->type)
    {
        case GET_IFACES:
            strcpy(outputBuf, "HANLDE");
            break;
    
        default:
            strcpy(outputBuf, "ERROR");
            break;
    }
}