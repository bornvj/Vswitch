#include "record.h"

record *records[RECORDS_SIZE] = {0};
size_t records_number = 0;

record* initRec(const unsigned char MAC[6], uint16_t VLAN, int INTERFACE)
{
    record *rec = malloc(sizeof(record));
    if (!rec)
        return NULL;

    memcpy(rec->MAC, MAC, sizeof(unsigned char) * 6);
    rec->INTERFACE = INTERFACE;
    rec->VLAN = VLAN;
    rec->last_seen = time(NULL);

    return rec;
}
    
record *mac_table_lookup(const unsigned char MAC[6], uint16_t VLAN_ID)
{
    for (size_t i = 0; i < RECORDS_SIZE; i++)
        if (records[i] && 
            memcmp(MAC, records[i]->MAC, 6) == 0 
            && VLAN_ID == (records[i])->VLAN)
            return records[i];

    return NULL;
}

void mac_table_learn(const unsigned char MAC[6], uint16_t VLAN_ID, int INTERFACE)
{
    size_t cursor = 0;

    while (records[cursor] && cursor < RECORDS_SIZE)
    {
        if (memcmp(MAC, records[cursor]->MAC, 6) == 0 && 
            records[cursor]->VLAN == VLAN_ID)
        {
            if (records[cursor]->INTERFACE != INTERFACE)
            {
                records[cursor]->INTERFACE = INTERFACE;
            }
            records[cursor]->last_seen = time(NULL);
            return;
        }
        cursor++;
    }

    record *rec = initRec(MAC, VLAN_ID, INTERFACE);
    if (!rec)
        return;

    cursor = 0;
    if (records_number == RECORDS_SIZE)
    {
        size_t oldest_pos = 0;
        time_t oldest_time = 0;

        while (cursor != RECORDS_SIZE)
        {
            if (records[cursor]->last_seen > oldest_time)
            {
                oldest_pos = oldest_pos;
                oldest_time = records[cursor]->last_seen;
            }
            cursor++;
        }
        free(records[oldest_pos]);
    }
    else
    {
        while (records[cursor]) {cursor++;}
        records_number++;
    }
    records[cursor] = rec;
}

void mac_table_age(time_t now)
{
    for (size_t i = 0; i < RECORDS_SIZE; i++)
    {
        if (records[i])
        {
            unsigned long diff = (unsigned long) difftime(now, records[i]->last_seen);
            if (diff >= MAX_DELAY)
            {
                free(records[i]);
                records[i] = NULL;

                records_number--;
            }
        }
    }
}

void mac_table_free()
{
    for (size_t i = 0; i < RECORDS_SIZE; i++)
    {
        if (records[i])
        {
            free(records[i]);
            records[i] = NULL;
        }
    }
}
