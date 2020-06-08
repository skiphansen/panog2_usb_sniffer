#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "gpio_defs.h"
#include "timer.h"
#include "pano_io.h"
#include "usb_sniffer_defs.h"

#define REG_WR(reg, wr_data)       *((volatile uint32_t *)(reg)) = (wr_data)
#define REG_RD(reg)                *((volatile uint32_t *)(reg))

bool ButtonJustPressed(void);
void LogBufferStatus(uint32_t Status);

//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
int main(int argc, char *argv[])
{
    int i;
    unsigned char Buf[256];
    int Id = 0;
    uint32_t Temp;
    uint32_t ReadBack;
    uint32_t Led;
    uint32_t UsbCmd = 3 << USB_BUFFER_CFG_SPEED_SHIFT;
    int Fast = 0;
    uint32_t CfgReg;
    uint32_t LastCfgReg;
    uint32_t Avail;
    uint32_t LastAvail;
    uint32_t Status;
    uint32_t LastStatus;

    printf("Hello pano world!\n");

    LastStatus = REG_RD(USB_BASE + USB_BUFFER_STS);
    LastAvail = REG_RD(USB_BASE + USB_FIFO_AVAILABLE);

    printf("USB_BUFFER_CFG: 0x%x\n",REG_RD(USB_BASE + USB_BUFFER_CFG));
    LogBufferStatus(LastStatus);
    printf("USB_FIFO_AVAILABLE: 0x%x\n",LastAvail);
    REG_WR(USB_BASE + USB_BUFFER_CFG,0);

// enable sniffer
    CfgReg = (1 << USB_BUFFER_CFG_ENABLED_SHIFT) |
             (1 << USB_BUFFER_CFG_IGNORE_IN_NAK_SHIFT) |
             (1 << USB_BUFFER_CFG_IGNORE_SOF_SHIFT) |
             (USB_SPEED_LS << USB_BUFFER_CFG_SPEED_SHIFT);

    printf("Setting USB_BUFFER_CFG to 0x%x\n",CfgReg);
    REG_WR(USB_BASE + USB_BUFFER_CFG,CfgReg);
    LastCfgReg = REG_RD(USB_BASE + USB_BUFFER_CFG);
    printf("USB_BUFFER_CFG: 0x%x\n",LastCfgReg);


// Set LED GPIO's to output
    Temp = REG_RD(GPIO_BASE + GPIO_DIRECTION);
    Temp |= GPIO_BIT_RED_LED|GPIO_BIT_GREEN_LED|GPIO_BIT_BLUE_LED;
    REG_WR(GPIO_BASE + GPIO_DIRECTION,Temp);

    Led = GPIO_BIT_RED_LED;
    for(; ; ) {
       Avail = REG_RD(USB_BASE + USB_FIFO_AVAILABLE);
       if(LastAvail != Avail) {
          union {
             uint32_t DataW;
             uint8_t DataB[4];
          } u;
          LastAvail = Avail;
          while(Avail > 0) {
             u.DataW = REG_RD(USB_BASE + USB_FIFO_READ);
             printf("%02x %02x %02x %02x\n",u.DataB[0],u.DataB[1],u.DataB[2],u.DataB[3]);
             Avail = REG_RD(USB_BASE + USB_FIFO_AVAILABLE);
          }
       }

       Status = REG_RD(USB_BASE + USB_BUFFER_STS);
       if(LastStatus != Status) {
          LastStatus = Status;
          LogBufferStatus(Status);
       }
       CfgReg = REG_RD(USB_BASE + USB_BUFFER_CFG);
       if(LastCfgReg != CfgReg) {
          LastCfgReg = CfgReg;
          printf("USB_BUFFER_CFG: 0x%x\n",LastCfgReg);
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

void LogBufferStatus(uint32_t Status)
{
   bool bFirst = true;
   printf("USB_BUFFER_STS: 0x%x",Status);
   if(Status != 0) {
      printf(" (");
      if(Status & (USB_BUFFER_STS_TRIG_MASK << USB_BUFFER_STS_TRIG_SHIFT)) {
         bFirst = false;
         printf("Triggered");
      }
      if(Status & (USB_BUFFER_STS_DATA_LOSS_MASK << USB_BUFFER_STS_DATA_LOSS_SHIFT)) {
         printf("%sDataLoss",bFirst ? "" : ", ");
      }
      printf(")");
   }
   printf("\n");
}

