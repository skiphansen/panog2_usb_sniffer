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
// #define VERBOSE_DEBUG_LOGGING 1
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

#include "usb_sniffer_defs.h"

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
err_t SendData(struct tcp_pcb *tpcb, u16_t BytesAcked);
void UpdateLedState(void);

#define MAX_ETH_FRAME_LEN     1518
int gRxCount;
uint8_t gRxBuf[MAX_ETH_FRAME_LEN];

uint8_t gOurMac[] = {MAC_ADR};
struct netif gNetif;
struct tcp_pcb *gTCP_pcb;
bool gSendRxBuf;
bool gConnected;
bool gHaveIpAdr;
int gBytesWritten;
int gBytes2Send;
int gBytesOutstanding;
int gAckDelay;
uint8_t *gSendBuf;
bool gFirst;

// This buffer must be larger than the largest size returned by tcp_sndbuf
// and it must be a multiple of 4 bytes
#define FIFO_BUF_LEN 1200
uint8_t gFifoTxBuf[FIFO_BUF_LEN];


typedef enum {
   LED_RED,
   LED_BLINK_RED,
   LED_GREEN,
   LED_BLINK_GREEN,
   LED_BLINK_BLUE,
   LED_BLUE
} LedState;

typedef enum {
   RCV_GET_CMD,
   RCV_GET_ADR,
   RCV_GET_LEN,
   RCV_GET_WRITE_DATA
} RcvState;

typedef enum {
   TX_IDLE,
   TX_SEND_BUF,
   TX_SEND_FIFO
} TxState;

TxState gTxState = TX_IDLE;
uint32_t *gTxBuf;
uint32_t gTxCount;      // in bytes


LedState gLedState = LED_BLINK_BLUE;

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
    u8_t bHaveIpAdr = 0;

    printf("Hello pano world!\n");

// Set LED GPIO's to output
    Temp = REG_RD(GPIO_BASE + GPIO_DIRECTION);
    Temp |= GPIO_BIT_RED_LED|GPIO_BIT_GREEN_LED|GPIO_BIT_BLUE_LED;
    REG_WR(GPIO_BASE + GPIO_DIRECTION,Temp);
    Led = GPIO_BIT_RED_LED;
// Stop any captures that are in progress
    REG_WR(USB_BASE + USB_BUFFER_CFG,0);
// Empty capture FIFO
    Temp = REG_RD(USB_BASE + USB_FIFO_STS) & USB_FIFO_STS_AVAIL_MASK;
    if(Temp != 0) {
       LOG("Clearing %d words from capture FIFO...",Temp);
       while((REG_RD(USB_BASE + USB_FIFO_STS) & USB_FIFO_STS_AVAIL_MASK) != 0) {
          REG_RD(USB_BASE + USB_FIFO_READ);
       }
       LOG_R("\n");
    }

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
             gHaveIpAdr = false;
             gConnected = false;
             UpdateLedState();
          }
       }

       if(bHaveIpAdr != dhcp_supplied_address(&gNetif)) {
          bHaveIpAdr = dhcp_supplied_address(&gNetif);
          if(bHaveIpAdr) {
             LOG("IP address assigned: %d.%d.%d.%d\n",
                 (gNetif.ip_addr.addr >> 0) & 0xff,
                 (gNetif.ip_addr.addr >> 8) & 0xff,
                 (gNetif.ip_addr.addr >> 16) & 0xff,
                 (gNetif.ip_addr.addr >> 24) & 0xff);
             gHaveIpAdr = true;
          }
          else {
             LOG("IP address lost\n");
             gHaveIpAdr = false;
          }
          UpdateLedState();
       }
       REG_WR(GPIO_BASE + GPIO_OUTPUT,Led);
       for(i = 0; i < 10; i++) {
          pano_netif_poll();
          timer_sleep(50);
       }

       switch(gLedState) {
          case LED_BLINK_GREEN:
          case LED_BLINK_RED:
          case LED_BLINK_BLUE:
             REG_WR(GPIO_BASE + GPIO_OUTPUT,0);
             break;
       }

       for(i = 0; i < 10; i++) {
          pano_netif_poll();
          timer_sleep(50);
       }
       switch(gLedState) {
          case LED_GREEN:
          case LED_BLINK_GREEN:
             Led = GPIO_BIT_GREEN_LED;
             break;

          case LED_RED:
          case LED_BLINK_RED:
             Led = GPIO_BIT_RED_LED;
             break;

          case LED_BLUE:
          case LED_BLINK_BLUE:
             Led = GPIO_BIT_BLUE_LED;
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

   LOG("called p: %p, tot_len: %d, len: %d: \n",p,p->tot_len,p->len);
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
         cp = (uint8_t *) p->payload;
         LOG("Next buf in chain tot_len: %d, len: %d: \n",p->tot_len,p->len);
         VLOG_HEX(cp,p->len);
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

     if((err = tcp_bind(gTCP_pcb,IP_ANY_TYPE,12345)) == ERR_OK) {
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
   if(err == ERR_OK && newpcb != NULL) {
      tcp_arg(newpcb,NULL);
      tcp_recv(newpcb,TcpRecv);
      tcp_err(newpcb,TcpError);
      tcp_poll(newpcb,TcpPoll,0);
      tcp_sent(newpcb,TcpSent);
      gConnected = true;
      UpdateLedState();
      ret_err = ERR_OK;
   }

   return ret_err;
}

#define CMD_NOP        0x0
#define CMD_WR         0x1
#define CMD_RD         0x2
#define CMD_RD_FIFO    0x3

/*
TcpRecv: called err: 0, pbuf->tot_len: 1, pbuf->len: 1:
01 CMD_WR
00 00 00 80  ADR
04 00 00 00 Len
00 00 00 00 
01 CMD_WR
04 00 00 80 
04 00 00 00 
00 00 00 00 
02 
04 00 00 80 adr 
04 00 00 00 len
*/
void ParseRxData(struct tcp_pcb *tpcb,uint8_t Byte)
{
   static int BytesNeeded;
   static RcvState State = RCV_GET_CMD;
   static uint8_t Cmd;
   static uint32_t Len;
   static uint32_t Adr;
   static uint32_t Value;
   static uint8_t *pBuf = &Cmd;

   switch(State) {
      case RCV_GET_CMD:
         Cmd = Byte;
         switch(Cmd) {
            case CMD_RD_FIFO:
               State = RCV_GET_LEN;
               pBuf = (uint8_t *) &Len;
               BytesNeeded = 4;
               break;

            default:
               State = RCV_GET_ADR;
               pBuf = (uint8_t *) &Adr;
               BytesNeeded = 4;
         }
         break;

      case RCV_GET_ADR:
         *pBuf++ = Byte;
         if(--BytesNeeded == 0) {
            State = RCV_GET_LEN;
            pBuf = (uint8_t *) &Len;
            BytesNeeded = 4;
         }
         break;

      case RCV_GET_LEN:
         *pBuf++ = Byte;
         if(--BytesNeeded == 0) {
            // Have Adr and Len, execute command
            switch(Cmd) {
               case CMD_WR:
                  if(Len != 4) {
                     ELOG("#%d: Internal error\n",__LINE__);
                     break;
                  }
                  LOG("Write %d bytes to 0x%04x\n",Len,Adr);
                  pBuf = (uint8_t *) &Value;
                  State = RCV_GET_WRITE_DATA;
                  BytesNeeded = Len;
                  gFirst = true;
                  break;

               case CMD_RD_FIFO:
                  LOG("Read %d bytes from FIFO\n",Len);
                  gBytes2Send = Len;
                  gTxState = TX_SEND_FIFO;
                  State = RCV_GET_CMD;
                  gAckDelay = gBytesOutstanding;
                  SendData(tpcb,0);
                  break;

               case CMD_RD:
                  State = RCV_GET_CMD;
                  Value = REG_RD(Adr);
                  LOG("Read %d bytes from 0x%04x = 0x%04x\n",Len,Adr,Value);
                  if(Value == 0 && Adr == (USB_BASE + USB_FIFO_STS)) {
                  // Special case for empty FIFO, a second before returning 
                  // data to keep from hammering the link
                     timer_sleep(1000);
                     LOG("Sleeping for a second ... ");
                     Value = REG_RD(Adr);
                     LOG_R("%d words available\n",Value);
                  }
                  *((uint32_t *) gFifoTxBuf) = Value;
                  gSendBuf = gFifoTxBuf;
                  gBytes2Send = Len;
                  gTxState = TX_SEND_BUF;
                  SendData(tpcb,0);
                  break;

               default:
                  ELOG("#%d: Internal error\n",__LINE__);
                  break;
            }
         }
         break;

      case RCV_GET_WRITE_DATA:
         *pBuf++ = Byte;
         if(--BytesNeeded == 0) {
            LOG("Write 0x%x to 0x%04x\n",Value,Adr);
            *((uint32_t *) Adr) = Value;
            Adr += 4;
            Len -= 4;
            if(Len > 0) {
               pBuf = (uint8_t *) &Value;
               BytesNeeded = 4;
            }
            else {
               State = RCV_GET_CMD;
            }
         }
         break;
   }
}

err_t TcpRecv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
   uint8_t *cp;
   int i;
   
   VLOG("called err: %d, p: %p\n",err,p);
   if(p != NULL) {
      LOG("called err: %d, pbuf->tot_len: %d, pbuf->len: %d: \n",err,
           p->tot_len,p->len);
      LOG_HEX(p->payload,p->len);
      cp = (uint8_t *) p->payload;
      for(i = 0; i < p->len; i++) {
         ParseRxData(tpcb,*cp++);
      }
      tcp_recved(tpcb, p->len);
      pbuf_free(p);
      VLOG("tcp_sndbuf: %d\n",tcp_sndbuf(tpcb));
   }
   else {
      LOG("connection closed\n");
      gConnected = false;
      UpdateLedState();

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

   VLOG("tpcb: %p\n",tpcb);

   return ERR_OK;
}

err_t SendData(struct tcp_pcb *tpcb, u16_t BytesAcked)
{
   static int WrOff;    // current offset into gFifoTxBuf filling
   static int RdOff;    // current offset into gFifoTxBuf reading
   err_t Err;
   int TcpSpace = tcp_sndbuf(tpcb);
   int Bytes2Send;
   uint32_t *p;
   int i;
   
   LOG("BytesAcked: %d, TcpSpace: %d, RdOff: %d, WrOff: %d\n",
       BytesAcked,TcpSpace,RdOff,WrOff);

   if(RdOff < 0 || RdOff >= FIFO_BUF_LEN) for(;;);
   if(WrOff < 0 || WrOff >= FIFO_BUF_LEN) for(;;);
      
   Bytes2Send = gBytes2Send - gBytesWritten;
   gBytesOutstanding -= BytesAcked;
   LOG("gBytesWritten: %d, Bytes2Send: %d\n",gBytesWritten,Bytes2Send);

   if(gTxState == TX_SEND_FIFO) {
      if(gAckDelay != 0) {
         if(BytesAcked > gAckDelay) {
            BytesAcked -= gAckDelay;
         }
         else {
         // Don't advance RdOff yet
            gAckDelay -= BytesAcked;
            BytesAcked = 0; 
         }
      }
      RdOff += BytesAcked;
      if(RdOff >= FIFO_BUF_LEN) {
         RdOff -= FIFO_BUF_LEN;
      }
      Bytes2Send &= ~3;
      LOG("RdOff: %d, Bytes2Send: %d\n",RdOff,Bytes2Send);
   }

   if(Bytes2Send > 0) {
      if(Bytes2Send > TcpSpace) {
         Bytes2Send = TcpSpace;
         LOG("tcpspace limits Bytes2Send: %d\n",Bytes2Send);
      }

      if(Bytes2Send > 0) {
         switch(gTxState) {
            case TX_SEND_BUF:
               LOG("Calling tcp_write with %d bytes:\n",Bytes2Send);
               VLOG_HEX(&gSendBuf[gBytesWritten],Bytes2Send);
               Err = tcp_write(tpcb,&gSendBuf[gBytesWritten],Bytes2Send,0);
               gBytesWritten += Bytes2Send;
               gBytesOutstanding += Bytes2Send;
               break;

            case TX_SEND_FIFO:
               if((Bytes2Send & 3) != 0) {
                  ELOG("#%d: Internal error\n",__LINE__);
               }
               if(WrOff == RdOff) {
                  WrOff = RdOff = 0;
                  LOG("Reset WrOff, RdOff\n");
               }

               if(WrOff + Bytes2Send >= FIFO_BUF_LEN) {
                  Bytes2Send = FIFO_BUF_LEN - WrOff;
                  LOG("buffer wrap limits Bytes2Send: %d\n",Bytes2Send);
               }
               p = (uint32_t *) &gFifoTxBuf[WrOff];
               for(i = 0; i < Bytes2Send; i += sizeof(uint32_t)) {
                  *p++ = REG_RD(USB_BASE + USB_FIFO_READ);
               }
               LOG("Calling tcp_write with %d bytes:\n",Bytes2Send);
               gBytesOutstanding += Bytes2Send;
               if(gFirst) {
                  gFirst = false;
                  LOG_HEX(&gFifoTxBuf[WrOff],Bytes2Send);
               }
               Err = tcp_write(tpcb,&gFifoTxBuf[WrOff],Bytes2Send,0);
               WrOff += Bytes2Send;
               gBytesWritten += Bytes2Send;
               if(WrOff > FIFO_BUF_LEN) {
                  ELOG("#%d: Internal error\n",__LINE__);
               }
               else if(WrOff == FIFO_BUF_LEN) {
                  WrOff = 0;
               }
               break;

            default:
               ELOG("#%d: Internal error, gTxState: %d\n",__LINE__,gTxState);
               break;
         }
      }
      else {
         LOG("Bytes2Send now zero\n");
      }
   }
   else {
      gTxState = TX_IDLE;
      gBytes2Send = 0;
      gBytesWritten = 0;
   }
   if(RdOff < 0 || RdOff >= FIFO_BUF_LEN) for(;;);
   if(WrOff < 0 || WrOff >= FIFO_BUF_LEN) for(;;);
}

err_t TcpSent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
   LOG("Called, tpcb: %p, gAckDelay: %d, len: %d\n",tpcb,gAckDelay,len);
   return SendData(tpcb,len);
}

// LED
// blinking blue  - No IP address
// blue           - IP address, no connection
// green          - client connected, capture not enabled or not trigger
// blinking green - capturing
// red            - overrun 
void UpdateLedState()
{
   if(!gHaveIpAdr) {
      gLedState = LED_BLINK_BLUE;
   }
   else if(!gConnected) {
      gLedState = LED_BLINK_GREEN;
   }
   else {
      gLedState = LED_GREEN;
   }
}

