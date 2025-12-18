#include "record.h"

mac_table* initMacTable(size_t capacity)
{
    mac_table *ret = malloc(sizeof(mac_table));
    if (!ret)
        return NULL;

    ret->records = calloc(capacity, sizeof(record*));;
    if (!ret->records)
        return NULL;

    ret->capacity = capacity;
    ret->count = 0;

    return ret;
}

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
    
record *mac_table_lookup(mac_table *mac_table, const unsigned char MAC[6], uint16_t VLAN)
{
    if (!mac_table)
        return NULL;

    for (size_t i = 0; i < mac_table->capacity; i++)
        if (mac_table->records[i] && 
            memcmp(MAC, (mac_table->records[i])->MAC, 6 * sizeof(unsigned char)) == 0 
            && VLAN == (mac_table->records[i])->VLAN)
            return mac_table->records[i];

    return NULL;
}

void mac_table_learn(mac_table *mac_table, const unsigned char MAC[6], uint16_t VLAN, int INTERFACE);

void mac_table_age(mac_table *mac_table, time_t now)
{
    if (!mac_table)
        return;

    for (size_t i = 0; i < mac_table->capacity; i++)
    {
        if (mac_table->records[i])
        {
            unsigned long diff = (unsigned long) difftime(now, mac_table->records[i]->last_seen);
            if (diff >= MAX_DELAY)
            {
                free(mac_table->records[i]);
                mac_table->records[i] = NULL;
            }
        }
    }
}

void mac_table_free(mac_table *mac_table)
{
    if (!mac_table)
        return;

    for (size_t i = 0; i < mac_table->capacity; i++)
    {
        if (mac_table->records[i])
        {
            free(mac_table->records[i]);
            mac_table->records[i] = NULL;
        }
    }

    free(mac_table->records);
    free(mac_table);
}
