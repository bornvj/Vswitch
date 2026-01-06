#pragma once

#include <time.h>
#include "interface.h"
#include "record.h"

#define MAX_IFACES 20

typedef struct 
{
    // interfaces
    struct iface ifaces[MAX_IFACES];
    size_t nbr_ifaces;

    // MAC table
    record *mac_table;
    size_t mac_table_size;

    // metadata
    time_t start_time;
} switch_ctx;