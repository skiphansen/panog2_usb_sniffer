#ifndef __USB_SNIFFER_H__
#define __USB_SNIFFER_H__

//--------------------------------------------------------------------
// Enums
//--------------------------------------------------------------------
typedef enum
{
   USB_SPEED_HS,
   USB_SPEED_FS,
   USB_SPEED_LS
} tUsbSpeed;

//--------------------------------------------------------------------
// Prototypes
//--------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

int usb_sniffer_init(char *IP);
int usb_sniffer_close(void);

int usb_sniffer_match_device(int dev, bool exclude);
int usb_sniffer_match_endpoint(int ep, bool exclude);
int usb_sniffer_drop_sof(int drop_sof);
int usb_sniffer_drop_in_nak(int drop_in_nak);
int usb_sniffer_continuous_mode(int enable);
int usb_sniffer_set_speed(tUsbSpeed speed);
int usb_sniffer_set_config(void);
int usb_sniffer_start(void);
int usb_sniffer_stop(void);
int usb_sniffer_triggered(void);
int usb_sniffer_wrapped(void);
int usb_sniffer_overrun(void);
uint32_t usb_sniffer_status(void);
int usb_sniffer_get_buffer(uint8_t *buffer, int buffer_size);
int usb_sniffer_read_buffer(uint8_t *buffer, int size);

#ifdef __cplusplus
}
#endif

#endif
