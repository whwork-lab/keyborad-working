#ifndef DHCP_DNS_HELPERS_H
#define DHCP_DNS_HELPERS_H

#include "lwip/ip_addr.h"

// Standard embedded DHCP state tracking structures
typedef struct dhcp_entry_t {
    uint8_t mac[6];
    ip4_addr_t ipaddr;
    uint32_t lease;
} dhcp_entry_t;

typedef struct dhcp_config_t {
    ip4_addr_t router;
    ip4_addr_t netmask;
    ip4_addr_t dns;
    ip4_addr_t start;
    int max_leases;
    dhcp_entry_t *leases;
} dhcp_config_t;

// Expose minimal DHCP/DNS functions derived from pico-examples
void dhcp_server_init(dhcp_config_t *config);
void dns_server_init(const ip4_addr_t *ipaddr);

#endif
