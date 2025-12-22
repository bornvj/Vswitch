#ifndef RECORD_H
#define RECORD_H

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

#define BUCKETS_SIZE UINT8_MAX
#define MAX_DELAY 600

typedef struct record
{
    unsigned char MAC[6];
    size_t INTERFACE;
    uint16_t VLAN;
    time_t last_seen;
} record;

typedef struct bucket
{
    record *rec;
    struct bucket *next;
} bucket;

extern bucket *mac_table[BUCKETS_SIZE];

bucket* bucket_init(record *rec);
record* initRec(const unsigned char MAC[6], uint16_t VLAN, size_t INTERFACE);

record *mac_table_lookup(const unsigned char MAC[6], uint16_t VLAN_ID);
void mac_table_learn(const unsigned char MAC[6], uint16_t VLAN_ID, size_t INTERFACE);
void mac_table_age(time_t now);

void mac_table_free(void);
void bucket_free(bucket *b);

#endif