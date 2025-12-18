#ifndef RECORD_H
#define RECORD_H

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

#define MAX_DELAY 600

typedef struct record
{
    unsigned char MAC[6];
    int INTERFACE;
    uint16_t VLAN;
    time_t last_seen;
} record;

typedef struct mac_table 
{
    record **records;
    size_t capacity;
    size_t count;
} mac_table;

mac_table* initMacTable(size_t capacity);

record* initRec(const unsigned char MAC[6], uint16_t VLAN, int INTERFACE);

record *mac_table_lookup(mac_table *mac_table, const unsigned char MAC[6], uint16_t VLAN);
void mac_table_learn(mac_table *mac_table, const unsigned char MAC[6], uint16_t VLAN, int INTERFACE);
void mac_table_age(mac_table *mac_table, time_t now);
void mac_table_free(mac_table *mac_table);

#endif