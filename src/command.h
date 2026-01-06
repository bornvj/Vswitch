#ifndef COMMAND_H
#define COMMAND_H

#include <stdlib.h>
#include "switch_ctx.h"

enum command_type
{
    ERROR,
    GET_IFACES, //  GET IFACE           -> ["if1", "if2", ...]
    GET_TRAFIC, //  GET TRAFIC IFACE    -> 
};

typedef struct command
{
    enum command_type type;
} command;

command *parseCommand(unsigned char *buf, size_t len);
void printCommand(command *cmd);
void handleCommand(command *cmd, char* outputBuf, switch_ctx ctx);

#endif