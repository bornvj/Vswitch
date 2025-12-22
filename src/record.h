#ifndef RECORD_H
#define RECORD_H

/**
 * @file record.h
 * @brief MAC table record definitions
 */


#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>

#define BUCKETS_SIZE UINT8_MAX  /**< Number of buckets in the MAC table (without the adjascent list) */
#define MAX_DELAY 600           /**< Max delay in second before a bucket is removed from the mac_table*/

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

/**
 * @brief Static MAC table, holding all the buckets
 */
extern bucket *mac_table[BUCKETS_SIZE];

bucket* bucket_init(record *rec);
record* initRec(const unsigned char MAC[6], uint16_t VLAN, size_t INTERFACE);

/**
 * @brief Looks in the mac_table if a record with then same (MAC, VLAN_ID) already exit and return it
 * 
 * @param MAC mac address
 * @param VLAN_ID vlan id
 * 
 * @return return a pointer to a rec if found
 */
record *mac_table_lookup(const unsigned char MAC[6], uint16_t VLAN_ID);
void mac_table_learn(const unsigned char MAC[6], uint16_t VLAN_ID, size_t INTERFACE);
void mac_table_age(time_t now);

void mac_table_free(void);
void bucket_free(bucket *b);

#endif