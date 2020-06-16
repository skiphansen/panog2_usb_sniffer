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
#include "usb_defs.h"
#include "usb_helpers.h"
#include "log_format.h"

#define REG_WR(reg, wr_data)       *((volatile uint32_t *)(reg)) = (wr_data)
#define REG_RD(reg)                *((volatile uint32_t *)(reg))

int log_sof(uint32_t value, int is_hs);
int log_rst(uint32_t value, int is_hs);
int log_token(uint32_t value);
int log_handshake(uint32_t value);
int log_data(uint32_t value, uint8_t *data, int length);

bool ButtonJustPressed(void);
void LogBufferStatus(uint32_t Status);
uint32_t GetCaptureData(void);

uint8_t  usb_data[MAX_PACKET_SIZE];

//-----------------------------------------------------------------
// main
//-----------------------------------------------------------------
int main(int argc, char *argv[])
{
    int i;
    uint32_t UsbCmd;
    uint32_t CfgReg;
    uint32_t LastCfgReg;
    uint32_t Avail;
    uint32_t LastAvail;
    uint32_t Status;
    uint32_t LastStatus;
    uint32_t value;
    uint32_t speed = USB_SPEED_LS;

    printf("Hello pano world!\n");

    do {
       CfgReg = REG_RD(USB_BASE + USB_BUFFER_CFG);
       LastStatus = REG_RD(USB_BASE + USB_BUFFER_STS);
       LastAvail = REG_RD(USB_BASE + USB_FIFO_STS);

       printf("USB_BUFFER_CFG: 0x%x\n",CfgReg);
       LogBufferStatus(LastStatus);
       printf("USB_FIFO_STS: 0x%x\n",LastAvail);

       if(CfgReg != 0) {
          printf("Setting USB_BUFFER_CFG to 0\n");
          REG_WR(USB_BASE + USB_BUFFER_CFG,0);
       }
    } while(CfgReg != 0);
// enable sniffer
    CfgReg = (speed << USB_BUFFER_CFG_SPEED_SHIFT)
#if 0
          | (1 << USB_BUFFER_CFG_IGNORE_IN_NAK_SHIFT)
          | (1 << USB_BUFFER_CFG_IGNORE_SOF_SHIFT)
#endif
    ;

    printf("Setting USB_BUFFER_CFG to 0x%x\n",CfgReg);
    REG_WR(USB_BASE + USB_BUFFER_CFG,CfgReg);

    CfgReg |= (1 << USB_BUFFER_CFG_ENABLED_SHIFT);

    printf("Setting USB_BUFFER_CFG to 0x%x\n",CfgReg);
    REG_WR(USB_BASE + USB_BUFFER_CFG,CfgReg);


#if 0
    for( ; ; ) {
       Avail = REG_RD(USB_BASE + USB_FIFO_STS);
       if(LastAvail != Avail) {
          union {
             uint32_t DataW;
             uint8_t DataB[4];
          } u;
          LastAvail = Avail;
          printf("USB_FIFO_STS: 0x%x\n",LastAvail);
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
#else
    for( ; ; ) {
       value = GetCaptureData();
       switch((value >> LOG_CTRL_TYPE_L) & LOG_CTRL_CYCLE_MASK) {
          case LOG_CTRL_TYPE_SOF:
             log_sof(value, speed == USB_SPEED_HS);
             break;

          case LOG_CTRL_TYPE_RST:
             log_rst(value, speed == USB_SPEED_HS);
             break;

          case LOG_CTRL_TYPE_TOKEN:
             log_token(value);
             break;

          case LOG_CTRL_TYPE_HSHAKE:
             log_handshake(value);
             break;

          case LOG_CTRL_TYPE_DATA: {
             uint32_t len = usb_get_data_length(value);
             uint32_t usb_idx = 0;
             uint32_t data;
             int j;

             for(i = 0; i < len; i+= 4) {
                data = GetCaptureData();

                for(j=0; j<4 && usb_idx < len; j++)
                   usb_data[usb_idx++] = data >> (8 * j);
             }
             log_data(value, usb_data, len);
             break;
          }

          default:
             printf("ERROR: Unknown ID %x\n", value);
       }
    }
#endif

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

uint32_t GetCaptureData()
{
   static uint32_t LastStatus = 0xffffffff;
   static uint32_t LastCfgReg = 0xffffffff;
   uint32_t CfgReg;
   uint32_t Status;
   uint32_t Data;
   uint32_t Avail;

   Avail = REG_RD(USB_BASE + USB_FIFO_STS);

   printf("Avail: 0x%x\n",Avail);
   do {
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
      Avail = REG_RD(USB_BASE + USB_FIFO_STS);
   } while(Avail == 0);

   Data = REG_RD(USB_BASE + USB_FIFO_READ);
   Avail = REG_RD(USB_BASE + USB_FIFO_STS);
   printf("%s: returning 0x%x, Avail: 0x%x\n",__FUNCTION__,Data,Avail);

   return Data;
}
