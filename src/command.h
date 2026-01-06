#ifndef COMMAND_H
#define COMMAND_H

#include <stdlib.h>

enum command_type
{
    ERROR,
    GET_IFACES, // [if1, if2, ...]
};

typedef struct command
{
    enum command_type type;
} command;

command *parseCommand(unsigned char *buf, size_t len);
void printCommand(command *cmd);
void handleCommand(command *cmd, char* outputBuf);

#endif