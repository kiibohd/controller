#ifndef _usb_mem_h_
#define _usb_mem_h_

#include <stdint.h>

typedef struct usb_packet_struct {
	uint16_t len;
	uint16_t index;
	struct usb_packet_struct *next;
	uint8_t buf[64];
} usb_packet_t;

usb_packet_t * usb_malloc(void);
void usb_free(usb_packet_t *p);




#endif
