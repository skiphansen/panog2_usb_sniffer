#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "gpio_defs.h"
#include "timer.h"
#include "pano_io.h"
#define DEBUG_LOGGING         1
#define VERBOSE_DEBUG_LOGGING 1
#include "log.h"
#include "eth_io.h"

/* lwIP core includes */
#include "lwip/opt.h"

#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/api.h"

#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/snmp.h"

/* lwIP netif includes */
#include "lwip/etharp.h"
#include "netif/ethernet.h"

#define REG_WR(reg, wr_data)       *((volatile uint32_t *)(reg)) = (wr_data)
#define REG_RD(reg)                *((volatile uint32_t *)(reg))

bool ButtonJustPressed(void);
void ClearRxFifo(void);
void init_default_netif(void);
void pano_netif_poll(void);
void netif_init(void);
err_t pano_netif_output(struct netif *netif, struct pbuf *p);
void TcpInit(void);
err_t TcpAccept(void *arg, struct tcp_pcb *newpcb, err_t err);
err_t TcpRecv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void TcpError(void *arg, err_t err);
err_t TcpPoll(void *arg, struct tcp_pcb *tpcb);
err_t TcpSent(void *arg, struct tcp_pcb *tpcb, u16_t len);

#define MAX_ETH_FRAME_LEN     1518
int gRxCount;
uint8_t gRxBuf[MAX_ETH_FRAME_LEN];

uint8_t gOurMac[] = {MAC_ADR};
struct netif gNetif;
struct tcp_pcb *gTCP_pcb;
bool gWelcomeSent;
bool gSendRxBuf;

//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
int main(int argc, char *argv[])
{
    int i;
    unsigned char Buf[256];
    int Id = 0;
    uint32_t Temp;
    uint32_t Led;
    uint32_t EthStatus = 0;
    uint32_t NewEthStatus;
    uint8_t Byte;
    uint16_t Count;

    printf("Hello pano world!\n");

// Set LED GPIO's to output
    Temp = REG_RD(GPIO_BASE + GPIO_DIRECTION);
    Temp |= GPIO_BIT_RED_LED|GPIO_BIT_GREEN_LED|GPIO_BIT_BLUE_LED;
    REG_WR(GPIO_BASE + GPIO_DIRECTION,Temp);
    Led = GPIO_BIT_RED_LED;

    lwip_init();
    init_default_netif();
    TcpInit();
    ClearRxFifo();

    for(; ; ) {
       NewEthStatus = ETH_STATUS & (ETH_STATUS_LINK_UP | ETH_STATUS_LINK_SPEED);
       if(EthStatus != NewEthStatus) {
          LOG_R("Ethernet Status: 0x%x\n",ETH_STATUS);
          EthStatus = NewEthStatus;
          LOG_R("Link is %s\n",
                 (EthStatus & ETH_STATUS_LINK_UP) ? "up" : "down");
          if(EthStatus & ETH_STATUS_LINK_UP) {
             LOG_R("Link speed: ");
             switch(EthStatus & ETH_STATUS_LINK_SPEED) {
                case SPEED_1000MBPS:
                   LOG_R("1g");
                   break;

                case SPEED_100MBPS:
                   LOG_R("100m");
                   break;

                case SPEED_10MBPS:
                   LOG_R("10m");
                   break;

                case SPEED_UNSPECIFIED:
                   LOG_R("?");
                   break;

                default:
                   LOG_R("WTF?");
                   break;
             }
             LOG_R("\n");
             netif_set_link_up(&gNetif);
          }
          else {
             netif_set_link_down(&gNetif);
          }
       }

       REG_WR(GPIO_BASE + GPIO_OUTPUT,Led);
       for(i = 0; i < 10; i++) {
          pano_netif_poll();
          timer_sleep(50);
       }
       REG_WR(GPIO_BASE + GPIO_OUTPUT,0);
       for(i = 0; i < 10; i++) {
          pano_netif_poll();
          timer_sleep(50);
       }
       switch(Led) {
          case GPIO_BIT_RED_LED:
             Led = GPIO_BIT_GREEN_LED;
             break;

          case GPIO_BIT_GREEN_LED:
             Led = GPIO_BIT_BLUE_LED;
             break;

          case GPIO_BIT_BLUE_LED:
             Led = GPIO_BIT_RED_LED;
             break;
       }
    }

    return 0;
}

bool ButtonJustPressed()
{
   static uint32_t ButtonLast = 3;
   uint32_t Temp;
   int Ret = 0;

   Temp = REG_RD(GPIO_BASE + GPIO_INPUT) & GPIO_BIT_PANO_BUTTON;
   if(ButtonLast != 3 && ButtonLast != Temp) {
      if(Temp == 0) {
         printf("Pano button pressed\n");
         Ret = 1;
      }
   }
   ButtonLast = Temp;

   return Ret;
}


void ClearRxFifo()
{
   int i;
   uint8_t Byte;

   if(!(ETH_STATUS & ETH_STATUS_RXEMPTY)) {
      ELOG("Clearing Rx FIFO\n");
      for(i = 0; i < 2048; i++) {
         if(ETH_STATUS & ETH_STATUS_RXEMPTY) {
            break;
         }
         gRxBuf[i % MAX_ETH_FRAME_LEN] = ETH_RX();
      }
      ELOG("FIFO %scleared after %d reads\n",i == 2048 ? "not " : "",i);
   }
}

void lwip_pano_assert(const char *msg, int line, const char *file);
void lwip_pano_assert(const char *msg, int line, const char *file)
{
   ALOG_R("Assertion \"%s\" failed %s#%d\n",msg,file,line);
   for( ; ; );
}


/**
 * @ingroup sys_time
 * Returns the current time in milliseconds,
 * may be the same as sys_jiffies or at least based on it.
 * Don't care for wraparound, this is only used for time diffs.
 * Not implementing this function means you cannot use some modules (e.g. TCP
 * timestamps, internal timeouts for NO_SYS==1).
 */
uint32_t sys_now(void);
uint32_t sys_now(void)
{
   return (uint32_t) timer_now();
}


unsigned int lwip_port_rand(void)
{
  return (unsigned int) rand();
}


err_t pano_netif_init(struct netif *netif);

err_t pano_netif_init(struct netif *netif)
{
   VLOG("%s: called\n",__FUNCTION__);

   netif->linkoutput = pano_netif_output;
   netif->output     = etharp_output;
   netif->mtu        = 1500;
   netif->flags      = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
   MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);

   memcpy(netif->hwaddr,gOurMac,ETH_HWADDR_LEN);
   netif->hwaddr_len = ETH_HWADDR_LEN;

   return ERR_OK;
}


void init_default_netif()
{
   err_t Err;

   do {
      if(netif_add(&gNetif,IP4_ADDR_ANY,IP4_ADDR_ANY,IP4_ADDR_ANY,NULL,pano_netif_init,netif_input) == NULL) {
         ELOG("netif_add failed\n");
         break;
      }
      gNetif.name[0] = 'e';
      gNetif.name[1] = 't';
      gNetif.hostname = "pano_usb_sniffer";
      netif_set_default(&gNetif);
      netif_set_up(&gNetif);
      if((Err = dhcp_start(&gNetif)) != ERR_OK) {
         ELOG("dhcp_start failed: %d\n",Err);
         break;
      }
   } while(false);
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
err_t pano_netif_output(struct netif *netif, struct pbuf *p)
{
   uint8_t *cp = (uint8_t *) p->payload;
   u16_t TxLen = p->tot_len;
   u16_t ThisBufLen = p->len;
   struct pbuf *pNext = p->next;
   int i;

   VLOG("called tot_len: %d, len: %d: \n",p->tot_len,p->len);
   VLOG_HEX(p->payload,p->len);

   ETH_TX = (uint8_t) ((TxLen >> 8) & 0xff);
   ETH_TX = (uint8_t) (TxLen & 0xff);
   for(i = 0; i < TxLen; i++) {
      if(ThisBufLen == 0) {
         p = p->next;
         if(p == NULL) {
            ELOG("Error - Ran out of buffers before data\n");
            break;
         }
         ThisBufLen = p->len;
         LOG("Next buf in chain tot_len: %d, len: %d: \n",p->tot_len,p->len);
         LOG_HEX(p->payload,p->len);
      }
      ETH_TX = *cp++;
      ThisBufLen--;
   }

   return ERR_OK;
}

void pano_netif_poll()
{
   struct pbuf *p = NULL;
   u16_t len;
   ssize_t readlen;
   uint16_t Count;
   uint8_t *cp;
   int i;

   if(!(ETH_STATUS & ETH_STATUS_RXEMPTY)) do {
      Count = (ETH_RX() << 8) + ETH_RX();
      if(Count == 0 || Count > MAX_ETH_FRAME_LEN) {
         ELOG("Invalid rx frame length %d\n",Count);
         ClearRxFifo();
      }
      len = (u16_t)Count;
      MIB2_STATS_NETIF_ADD(&gNetif,ifinoctets,len);

      /* We allocate a pbuf chain of pbufs from the pool. */
      p = pbuf_alloc(PBUF_RAW,len,PBUF_POOL);
      if(p != NULL) {
         cp = (uint8_t *) p->payload;
         *cp = ETH_RX();
#ifdef MIB2_STATS
         if((*cp & 0x01) == 0) {
           MIB2_STATS_NETIF_INC(&gNetif,ifinucastpkts);
         }
         else {
           MIB2_STATS_NETIF_INC(&gNetif,ifinnucastpkts);
         }
#endif
         cp++;
         for(i = 1; i < Count; i++) {
            *cp++ = ETH_RX();
         }
         cp = (uint8_t *) p->payload;

         VLOG_R("Rx #%d: Read %d (0x%x) bytes from Rx Fifo:\n",gRxCount,Count,Count);
         VLOG_HEX(p->payload,p->len);
         if(gNetif.input(p,&gNetif) != ERR_OK) {
           LWIP_DEBUGF(NETIF_DEBUG,("%s: netif input error\n",__FUNCTION__));
           pbuf_free(p);
         }
      }
      else {
         /* drop packet(); */
         MIB2_STATS_NETIF_INC(gNetif,ifindiscards);
         LWIP_DEBUGF(NETIF_DEBUG,("%s: could not allocate pbuf\n",__FUNCTION__));
         for(i = 0; i < Count; i++) {
            (void) ETH_RX();
         }
      }
   } while(false);
   /* Cyclic lwIP timers check */
   sys_check_timeouts();

}


void TcpInit()
{
   err_t err;
   if((gTCP_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY)) != NULL) {

     if((err = tcp_bind(gTCP_pcb,IP_ANY_TYPE,23)) == ERR_OK) {
       gTCP_pcb = tcp_listen_with_backlog(gTCP_pcb,1);
       tcp_accept(gTCP_pcb,TcpAccept);
     }
     else {
        ELOG("tcp_bind failed: %d\n",err);
     }
   }
   else {
     ELOG("tcp_new_ip_type failed\n");
   }
}

err_t TcpAccept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
   err_t ret_err = ERR_VAL;
   err_t Err;
   struct tcpecho_raw_state *es;

   VLOG("Called, err: %d, newpcb: %p\n",err,newpcb);
   gWelcomeSent = false;
   if(err == ERR_OK && newpcb != NULL) {
      tcp_arg(newpcb,NULL);
      tcp_recv(newpcb,TcpRecv);
      tcp_err(newpcb,TcpError);
      tcp_poll(newpcb,TcpPoll,0);
      tcp_sent(newpcb,TcpSent);
      ret_err = ERR_OK;
   }

   return ret_err;
}

err_t TcpRecv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   uint8_t *cp;
   int i;

   VLOG("called err: %d, p: %p\n",err,p);
   if(p != NULL) {
      VLOG("called err: %d, pbuf->tot_len: %d, pbuf->len: %d: \n",err,
           p->tot_len,p->len);
      VLOG_HEX(p->payload,p->len);
      cp = (uint8_t *) p->payload;
      for(i = 0; i < p->len; i++) {
         if(*cp == 0xff) {
         // Skip telnet command
            cp += 3;
            i += 2;
            if(i + 1 > p->len) {
               ELOG("Internal error\n");
            }
         }
         else {
            if(*cp == '\r') {
               gSendRxBuf = true;
            }

            gRxBuf[gRxCount] = *cp++;
            if(gRxCount < sizeof(gRxBuf) - 1) {
               gRxCount++;
            }
         }
      }
      tcp_recved(tpcb, p->len);
      pbuf_free(p);

      VLOG("tcp_sndbuf: %d\n",tcp_sndbuf(tpcb));
   }
   else {
      VLOG("connection closed\n");
   }
   return ERR_OK;
}

void TcpError(void *arg, err_t err)
{
   ELOG("Called, err: %d\n",err);
}

err_t TcpPoll(void *arg, struct tcp_pcb *tpcb)
{
   err_t Err;
   static const char WelcomeMsg[] = "Welcome to the Pano world via TCP/IP!\r\n";

   VLOG("tpcb: %p\n",tpcb);
   if(!gWelcomeSent) {
      gWelcomeSent = true;
      LOG("calling tcp_write for welcome msg\n");
      Err = tcp_write(tpcb,WelcomeMsg,sizeof(WelcomeMsg)-1,TCP_WRITE_FLAG_COPY);
      if(Err != ERR_OK) {
         ELOG("tcp_write failed - %d\n",Err);
      }
   }
   else if(gSendRxBuf) {
      gSendRxBuf = false;
      LOG("calling tcp_write with RxBuf\n");
      VLOG_HEX(gRxBuf,gRxCount);
      Err = tcp_write(tpcb,gRxBuf,gRxCount,TCP_WRITE_FLAG_COPY);
      gRxCount = 0;
      if(Err != ERR_OK) {
         ELOG("tcp_write failed - %d\n",Err);
      }
   }
   return ERR_OK;
}

err_t TcpSent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
   LOG("Called, tpcb: %p, len: %d\n",tpcb,len);
}

