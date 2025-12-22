#ifndef TOOLS_H
#define TOOLS_H

#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "interface.h"

uint8_t hash(const unsigned char MAC[6], uint16_t vlan);
int mac_equal(const unsigned char MAC1[6], const unsigned char MAC2[6]);
void print_mac_table(struct iface *ifaces, size_t iface_nbr, time_t start_time);

#endif