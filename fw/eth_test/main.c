#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "gpio_defs.h"
#include "timer.h"
#include "pano_io.h"
#include "eth_io.h"

#define REG_WR(reg, wr_data)       *((volatile uint32_t *)(reg)) = (wr_data)
#define REG_RD(reg)                *((volatile uint32_t *)(reg))

bool ButtonJustPressed(void);
void ClearRxFifo(void);

#define MAX_ETH_FRAME_LEN     1518
int gRxCount;
uint8_t gRxBuf[MAX_ETH_FRAME_LEN];
uint8_t gOurMac[] = {MAC_ADR};

uint8_t gTxBuf[] = {
   0xff,0xff,0xff,0xff,0xff,0xff,
   MAC_ADR,
   0x88,0x99,0x03,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
   't','e','s','t'
};



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
    int Fast = 0;
    uint32_t EthStatus = 0;
    uint32_t NewEthStatus;
    uint8_t Byte;
    uint16_t Count;

    printf("Hello pano world!\n");
    printf("Click the pano button to change the LED blink rate.\n");

// Set LED GPIO's to output
    Temp = REG_RD(GPIO_BASE + GPIO_DIRECTION);
    Temp |= GPIO_BIT_RED_LED|GPIO_BIT_GREEN_LED|GPIO_BIT_BLUE_LED;
    REG_WR(GPIO_BASE + GPIO_DIRECTION,Temp);

    Led = GPIO_BIT_RED_LED;

    ClearRxFifo();

    for(; ; ) {
       NewEthStatus = ETH_STATUS & (ETH_STATUS_LINK_UP | ETH_STATUS_LINK_SPEED);
       if(EthStatus != NewEthStatus) {
          printf("Ethernet Status: 0x%x\n",ETH_STATUS);
          EthStatus = NewEthStatus;
          printf("Link is %s\n",
                 (EthStatus & ETH_STATUS_LINK_UP) ? "up" : "down");
          printf("Link speed: ");
          switch(EthStatus & ETH_STATUS_LINK_SPEED) {
             case SPEED_1000MBPS:
                printf("1g");
                break;

             case SPEED_100MBPS:
                printf("100m");
                break;

             case SPEED_10MBPS:
                printf("10m");
                break;

             case SPEED_UNSPECIFIED:
                printf("?");
                break;

             default:
                printf("WTF?");
                break;
          }
          printf("\n");
       }
       while(!(ETH_STATUS & ETH_STATUS_RXEMPTY)) {
          Count = (ETH_RX() << 8) + ETH_RX();
          if(Count < sizeof(gRxBuf)) {
             gRxCount++;
             for(i = 0; i < Count; i++) {
                gRxBuf[i] = ETH_RX();
             }
             printf("%d: Read %d (0x%x) bytes from Rx Fifo:",gRxCount,Count,
                    Count);
             for(i = 0; i < Count; i++) {
                if((i & 0xf) == 0) {
                   printf("\n");
                }
                printf("0x%02x ",gRxBuf[i]);
             }
             printf("\n");
             if(!(ETH_STATUS & ETH_STATUS_RXEMPTY)) {
                printf("Another RX frame is waiting\n");
             }
          }
          else {
             printf("Invalid data length %d (0x%x)\n",Count,Count);
             ClearRxFifo();
          }
       }
       if(ButtonJustPressed()) {
          printf("Sending test frame, eth status: 0x%x\n",ETH_STATUS);
          ETH_TX = (uint8_t) ((sizeof(gTxBuf) >> 8) & 0xff);
          ETH_TX = (uint8_t) (sizeof(gTxBuf) & 0xff);
          for(i = 0; i < sizeof(gTxBuf); i++) {
             ETH_TX = gTxBuf[i];
          }
          printf("eth status: 0x%x\n",ETH_STATUS);
       }
       REG_WR(GPIO_BASE + GPIO_OUTPUT,Led);
       for(i = 0; i < (Fast ? 3 : 10); i++) {
          timer_sleep(50);
       }
       REG_WR(GPIO_BASE + GPIO_OUTPUT,0);
       for(i = 0; i < (Fast ? 3 : 10); i++) {
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
      printf("Clearing Rx FIFO\n");
      for(i = 0; i < 2048; i++) {
         if(ETH_STATUS & ETH_STATUS_RXEMPTY) {
            break;
         }
         gRxBuf[i % MAX_ETH_FRAME_LEN] = ETH_RX();
      }
      printf("FIFO %scleared after %d reads\n",i == 2048 ? "not " : "",i);
   }
}
