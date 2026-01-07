#ifndef RECORD_H
#define RECORD_H

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

#define MAX_DELAY 600           /**< Max delay in second before a bucket is removed from the mac_table*/
#define BUCKETS_SIZE UINT8_MAX  /* Number of buckets in the MAC table (without the adjascent list) */

/**
 * @brief Record in the mac table. Contains data about a MAC input
 */
typedef struct record
{
    unsigned char MAC[6];
    size_t INTERFACE;
    uint16_t VLAN;
    time_t last_seen;
} record;

/**
 * @brief Holder for record in the mac_table
 */
typedef struct bucket
{
    record *rec;
    struct bucket *next;
} bucket;

bucket* bucket_init(record *rec);
record* initRec(const unsigned char MAC[6], uint16_t VLAN, size_t INTERFACE);

record *mac_table_lookup(bucket *mac_table[BUCKETS_SIZE], const unsigned char MAC[6], uint16_t VLAN_ID);
void mac_table_learn(bucket *mac_table[BUCKETS_SIZE], const unsigned char MAC[6], uint16_t VLAN_ID, size_t INTERFACE);
void mac_table_age(bucket *mac_table[BUCKETS_SIZE], time_t now);

void mac_table_free(bucket *mac_table[BUCKETS_SIZE]);
void bucket_free(bucket *b);

#endif