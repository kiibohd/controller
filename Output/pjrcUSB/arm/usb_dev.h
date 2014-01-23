#ifndef _usb_dev_h_
#define _usb_dev_h_

// This header is NOT meant to be included when compiling
// user sketches in Arduino.  The low-level functions
// provided by usb_dev.c are meant to be called only by
// code which provides higher-level interfaces to the user.

#include "usb_mem.h"
#include "usb_desc.h"

void usb_init(void);
uint8_t usb_configured(void);		// is the USB port configured
void usb_isr(void);
usb_packet_t *usb_rx(uint32_t endpoint);
uint32_t usb_rx_byte_count(uint32_t endpoint);
uint32_t usb_tx_byte_count(uint32_t endpoint);
uint32_t usb_tx_packet_count(uint32_t endpoint);
void usb_tx(uint32_t endpoint, usb_packet_t *packet);
void usb_tx_isr(uint32_t endpoint, usb_packet_t *packet);

void usb_device_reload();

extern volatile uint8_t usb_configuration;

#ifdef CDC_DATA_INTERFACE
extern uint8_t usb_cdc_line_coding[7];
extern volatile uint8_t usb_cdc_line_rtsdtr;
extern volatile uint8_t usb_cdc_transmit_flush_timer;
extern void usb_serial_flush_callback(void);
#endif


#endif

