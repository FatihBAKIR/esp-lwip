#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "c_types.h"
#include "eagle_soc.h"
#include "arch/cc.h"

// Don't change:
#define NO_SYS                              1
#define NO_SYS_NO_TIMERS                    0
#define LWIP_NETIF_TX_SINGLE_PBUF           1
#define LWIP_ESP                            1
#define PBUF_RSV_FOR_WLAN                   1
#define ICACHE_FLASH                        1
#define EBUF_LWIP                           1
#define LWIP_NETIF_HOSTNAME                 1
#define LWIP_CALLBACK_API                   1

// Leave unchanged unless you really know what you're doing:
#define MEM_ALIGNMENT                       4
#define TCP_MSS                             536
#define TCP_SND_BUF                         (2*TCP_MSS)
#define MEMP_MEM_MALLOC                     1
#define MEM_LIBC_MALLOC                     1

#define MEMP_NUM_TCP_PCB                    (*(volatile uint32*)0x600011FC)
#if 0
#define TCP_WND                             (*(volatile uint32*)0x600011F0)
#define TCP_MAXRTX                          (*(volatile uint32*)0x600011E8)
#define TCP_SYNMAXRTX                       (*(volatile uint32*)0x600011E4)
#endif
#define DHCP_MAXRTX                         (*(volatile uint32*)0x600011E0)

#define LWIP_RAND()                         rand()

#if MEM_LIBC_MALLOC
#define mem_free                            vPortFree
#define mem_malloc                          pvPortMalloc
#define mem_calloc                          pvPortCalloc
#endif

#define MEMCPY(dst,src,len)                 os_memcpy(dst,src,len)
#define SMEMCPY(dst,src,len)                os_memcpy(dst,src,len)

static inline u32_t sys_now(void)
{
    return NOW()/(TIMER_CLK_FREQ/1000);
}

// Required:
#define LWIP_DHCP                           1
#define LWIP_DNS                            1

// Optional:
#define LWIP_IGMP                           1
#define LWIP_NETIF_LOOPBACK                 0
#define LWIP_HAVE_LOOPIF                    0

// Tweakable:
#define ESP_TIMEWAIT_THRESHOLD              10000

#define LWIP_NETCONN                        0
#define LWIP_SOCKET                         0
#define MEMP_NUM_SYS_TIMEOUT                8

#define ARP_QUEUEING                        1
#define ETHARP_TRUST_IP_MAC                 1
#define IP_FRAG                             0
#define IP_REASSEMBLY                       0
#define IP_FRAG_USES_STATIC_BUF             1
#define TCP_QUEUE_OOSEQ                     1

#define LWIP_TCP_KEEPALIVE                  1
#define LWIP_STATS                          0

#ifdef LWIP_OUR_IF
  #define LWIP_NETIF_HOSTNAME_PREFIX        "esp8266-"
#endif

// #define LWIP_DEBUG
// #define IP_DEBUG                            LWIP_DBG_ON
#define LWIP_DBG_TYPES_ON                   LWIP_DBG_OFF

#define MEM_SIZE                            16000
#define MEMP_SEPARATE_POOLS                 1
#define MEMP_SANITY_CHECK                   1
#define MEMP_NUM_PBUF                       10
#define MEMP_NUM_TCP_PCB_LISTEN             2
#define MEMP_NUM_REASSDATA                  0
#define MEMP_NUM_FRAG_PBUF                  0
#define MEMP_NUM_ARP_QUEUE                  10
#define MEMP_NUM_NETBUF                     0
#define MEMP_NUM_NETCONN                    0
#define MEMP_NUM_TCPIP_MSG_API              4
#define MEMP_NUM_TCPIP_MSG_INPKT            4
#define MEMP_NUM_SNMP_NODE                  0
#define MEMP_NUM_SNMP_ROOTNODE              0
#define MEMP_NUM_SNMP_VARBIND               0
#define MEMP_NUM_SNMP_VALUE                 0
#define MEMP_NUM_NETDB                      0
#define MEMP_NUM_LOCALHOSTLIST              0
#define MEMP_NUM_PPPOE_INTERFACES           0
#define PBUF_POOL_SIZE                      10
#define SNMP_CONCURRENT_REQUESTS            0
#define SNMP_TRAP_DESTINATIONS              0
#define SNMP_SAFE_REQUESTS                  0
#define TCP_MAXRTO                          10
#define TCP_MINRTO                          2

#endif /* __LWIP_OPT_H__ */
