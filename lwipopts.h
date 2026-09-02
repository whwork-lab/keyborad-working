#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

#define NO_SYS                          1
#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0
#define LWIP_NETIF_HOSTNAME             1

#define MEM_ALIGNMENT                   4
#define SYS_LIGHTWEIGHT_PROT            1

#define LWIP_RAW                        1
#define LWIP_UDP                        1
#define LWIP_TCP                        1

// Explicitly downscale memory flags to prevent stack exhaustion on bare metal
#define TCP_MSS                         536
#define TCP_WND                         (4 * TCP_MSS)
#define TCP_SND_BUF                     (4 * TCP_MSS)

#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_ICMP                       1
#define LWIP_DHCP                       1

#endif
