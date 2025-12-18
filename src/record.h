#ifndef RECORD_H
#define RECORD_H

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

#define RECORDS_SIZE 1024
#define MAX_DELAY 600

typedef struct record
{
    unsigned char MAC[6];
    int INTERFACE;
    uint16_t VLAN;
    time_t last_seen;
} record;

extern record *records[RECORDS_SIZE];
extern size_t records_number;

record* initRec(const unsigned char MAC[6], uint16_t VLAN, int INTERFACE);

record *mac_table_lookup(const unsigned char MAC[6], uint16_t VLAN_ID);
void mac_table_learn(const unsigned char MAC[6], uint16_t VLAN_ID, int INTERFACE);
void mac_table_age(time_t now);
void mac_table_free(void);

#endif