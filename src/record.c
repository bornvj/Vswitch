#include "tools.h"
#include "record.h"

bucket *mac_table[BUCKETS_SIZE] = {0};

bucket* bucket_init(record *rec)
{
    if (!rec)
        return NULL;

    bucket *ret = malloc(sizeof(bucket));
    
    if (!ret)
        return NULL;

    ret->rec = rec;
    ret->next = NULL;

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
    
record *mac_table_lookup(const unsigned char MAC[6], uint16_t VLAN_ID)
{
    uint8_t key = hash(MAC, VLAN_ID);

    bucket *current = mac_table[key];

    while (current)
    {
        if (mac_equal(current->rec->MAC, MAC) && current->rec->VLAN == VLAN_ID)
            return current->rec;
        current = current->next;
    }

    return NULL;
}

void mac_table_learn(const unsigned char MAC[6], uint16_t VLAN_ID, int INTERFACE)
{
    uint8_t key = hash(MAC, VLAN_ID);

    bucket *current = mac_table[key];

    if (!current)
    {
        mac_table[key] = bucket_init(initRec(MAC, VLAN_ID, INTERFACE));
        return;
    }

    bucket *prev = NULL;

    while (current) 
    {
        if (mac_equal(current->rec->MAC, MAC) && current->rec->VLAN == VLAN_ID) 
        {

            if (current->rec->INTERFACE != INTERFACE)
                current->rec->INTERFACE = INTERFACE;
            current->rec->last_seen = time(NULL);
            return;
        }
        prev = current;
        current = current->next;
    }

    if (!prev)
        mac_table[key] = bucket_init(initRec(MAC, VLAN_ID, INTERFACE));
    else 
        prev->next = bucket_init(initRec(MAC, VLAN_ID, INTERFACE));
}


void mac_table_age_aux(bucket *b, time_t now)
{
    if (!b || !b->next)
        return;

    while ((now - b->next->rec->last_seen) > MAX_DELAY)
    {
        bucket *tmp = b->next->next;
        bucket_free(b->next);
        b->next = tmp;
    }

    mac_table_age_aux(b->next, now);
}

void mac_table_age(time_t now)
{
    for (size_t i = 0; i < BUCKETS_SIZE; i++)
    {
        if (mac_table[i])
        {
            bucket *current = mac_table[i];

            while ((now - current->rec->last_seen) > MAX_DELAY)
            {
                mac_table[i] = current->next;
                bucket_free(current);
                current = mac_table[i];
            }
            mac_table_age_aux(current, now);
        }
    }
}

void mac_table_free_aux(bucket *b)
{
    if (!b)
        return;

    mac_table_free_aux(b->next);

    bucket_free(b);
}

void mac_table_free()
{
    for (size_t i = 0; i < BUCKETS_SIZE; i++)
        mac_table_free_aux(mac_table[i]);

}

void bucket_free(bucket *b)
{
    if (!b)
        return;

    if (b->rec)
        free(b->rec);
    free(b);
}