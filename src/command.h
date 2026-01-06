#ifndef COMMAND_H
#define COMMAND_H

#include <stdlib.h>
#include "switch_ctx.h"

enum command_type
{
    ERROR,
    GET_IFACES,
    /* GET IFACES
    {
        "ifaces:" ["if1", "if2", ...]
    }        
    */
    GET_TRAFIC,
    /* GET TRAFIC
    {
        "rx_frames": value,
        "rx_bytes": value,
        "tx_frames": value,
        "tx_bytes": value
    }
    */
};

typedef struct command
{
    enum command_type type;
    char ifname[32];
} command;

command *parseCommand(unsigned char *buf, size_t len);
void printCommand(command *cmd);
void handleCommand(command *cmd, char* outputBuf, switch_ctx ctx);

#endif