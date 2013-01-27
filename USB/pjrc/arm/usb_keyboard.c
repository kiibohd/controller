#include "usb_dev.h"
#include "usb_keyboard.h"
#include <Lib/USBLib.h>
#include <string.h> // for memcpy()


// Maximum number of transmit packets to queue so we don't starve other endpoints for memory
#define TX_PACKET_LIMIT 4

static uint8_t transmit_previous_timeout=0;

// When the PC isn't listening, how long do we wait before discarding data?
#define TX_TIMEOUT_MSEC 50

#if F_CPU == 96000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 596)
#elif F_CPU == 48000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 428)
#elif F_CPU == 24000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 262)
#endif


// send the contents of keyboard_keys and keyboard_modifier_keys
uint8_t usb_keyboard_send(void)
{
	uint32_t wait_count=0;
	usb_packet_t *tx_packet;

	while (1) {
		if (!usb_configuration) {
			return -1;
		}
		if (usb_tx_packet_count(KEYBOARD_ENDPOINT) < TX_PACKET_LIMIT) {
			tx_packet = usb_malloc();
			if (tx_packet) break;
		}
		if (++wait_count > TX_TIMEOUT || transmit_previous_timeout) {
			transmit_previous_timeout = 1;
			return -1;
		}
		yield();
	}
	*(tx_packet->buf) = USBKeys_Modifiers;
	*(tx_packet->buf + 1) = 0;
	memcpy(tx_packet->buf + 2, USBKeys_Array, USB_MAX_KEY_SEND);
	tx_packet->len = 8;
	usb_tx(KEYBOARD_ENDPOINT, tx_packet);

	return 0;
}

