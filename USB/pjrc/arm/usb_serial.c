#include "usb_dev.h"
#include "usb_serial.h"
#include <Lib/USBLib.h>

// defined by usb_dev.h -> usb_desc.h
#if defined(CDC_STATUS_INTERFACE) && defined(CDC_DATA_INTERFACE)

uint8_t usb_cdc_line_coding[7];
volatile uint8_t usb_cdc_line_rtsdtr=0;
volatile uint8_t usb_cdc_transmit_flush_timer=0;

static usb_packet_t *rx_packet=NULL;
static usb_packet_t *tx_packet=NULL;
static volatile uint8_t tx_noautoflush=0;

#define TRANSMIT_FLUSH_TIMEOUT	5   /* in milliseconds */

static void usb_serial_receive(void)
{
	if (!usb_configuration) return;
	if (rx_packet) return;
	while (1) {
		rx_packet = usb_rx(CDC_RX_ENDPOINT);
		if (rx_packet == NULL) return;
		if (rx_packet->len > 0) return;
		usb_free(rx_packet);
		rx_packet = NULL;
	}
}

// get the next character, or -1 if nothing received
int usb_serial_getchar(void)
{
	unsigned int i;
	int c;

	usb_serial_receive();
	if (!rx_packet) return -1;
	i = rx_packet->index;
	c = rx_packet->buf[i++];
	if (i >= rx_packet->len) {
		usb_free(rx_packet);
		rx_packet = NULL;
	} else {
		rx_packet->index = i;
	}
	return c;
}

// peek at the next character, or -1 if nothing received
int usb_serial_peekchar(void)
{
	usb_serial_receive();
	if (!rx_packet) return -1;
	return rx_packet->buf[rx_packet->index];
}

// number of bytes available in the receive buffer
int usb_serial_available(void)
{
	int count=0;

	if (usb_configuration) {
		count = usb_rx_byte_count(CDC_RX_ENDPOINT);
	}
	if (rx_packet) count += rx_packet->len - rx_packet->index;
	return count;
}

// discard any buffered input
void usb_serial_flush_input(void)
{
	usb_packet_t *rx;

	if (!usb_configuration) return;
	if (rx_packet) {
		usb_free(rx_packet);
		rx_packet = NULL;
	}
	while (1) {
		rx = usb_rx(CDC_RX_ENDPOINT);
		if (!rx) break;
		usb_free(rx);
	}
}

// Maximum number of transmit packets to queue so we don't starve other endpoints for memory
#define TX_PACKET_LIMIT 8

// When the PC isn't listening, how long do we wait before discarding data?  If this is
// too short, we risk losing data during the stalls that are common with ordinary desktop
// software.  If it's too long, we stall the user's program when no software is running.
#define TX_TIMEOUT_MSEC 70

#if F_CPU == 96000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 596)
#elif F_CPU == 48000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 428)
#elif F_CPU == 24000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 262)
#endif

// When we've suffered the transmit timeout, don't wait again until the computer
// begins accepting data.  If no software is running to receive, we'll just discard
// data as rapidly as Serial.print() can generate it, until there's something to
// actually receive it.
static uint8_t transmit_previous_timeout=0;


// transmit a character.  0 returned on success, -1 on error
int usb_serial_putchar(uint8_t c)
{
#if 1
	return usb_serial_write(&c, 1);
#endif
#if 0
	uint32_t wait_count;

	tx_noautoflush = 1;
	if (!tx_packet) {
		wait_count = 0;
		while (1) {
			if (!usb_configuration) {
				tx_noautoflush = 0;
				return -1;
			}
			if (usb_tx_packet_count(CDC_TX_ENDPOINT) < TX_PACKET_LIMIT) {
				tx_noautoflush = 1;
				tx_packet = usb_malloc();
				if (tx_packet) break;
				tx_noautoflush = 0;
			}
			if (++wait_count > TX_TIMEOUT || transmit_previous_timeout) {
				transmit_previous_timeout = 1;
				return -1;
			}
		}
	}
	transmit_previous_timeout = 0;
	tx_packet->buf[tx_packet->index++] = c;
	if (tx_packet->index < CDC_TX_SIZE) {
		usb_cdc_transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
	} else {
		tx_packet->len = CDC_TX_SIZE;
		usb_cdc_transmit_flush_timer = 0;
		usb_tx(CDC_TX_ENDPOINT, tx_packet);
		tx_packet = NULL;
	}
	tx_noautoflush = 0;
	return 0;
#endif
}


int usb_serial_write(const void *buffer, uint32_t size)
{
#if 1
	uint32_t len;
	uint32_t wait_count;
	const uint8_t *src = (const uint8_t *)buffer;
	uint8_t *dest;

	tx_noautoflush = 1;
	while (size > 0) {
		if (!tx_packet) {
			wait_count = 0;
			while (1) {
				if (!usb_configuration) {
					tx_noautoflush = 0;
					return -1;
				}
				if (usb_tx_packet_count(CDC_TX_ENDPOINT) < TX_PACKET_LIMIT) {
					tx_noautoflush = 1;
					tx_packet = usb_malloc();
					if (tx_packet) break;
					tx_noautoflush = 0;
				}
				if (++wait_count > TX_TIMEOUT || transmit_previous_timeout) {
					transmit_previous_timeout = 1;
					return -1;
				}
				yield();
			}
		}
		transmit_previous_timeout = 0;
		len = CDC_TX_SIZE - tx_packet->index;
		if (len > size) len = size;
		dest = tx_packet->buf + tx_packet->index;
		tx_packet->index += len;
		size -= len;
		while (len-- > 0) *dest++ = *src++;
		if (tx_packet->index < CDC_TX_SIZE) {
			usb_cdc_transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
		} else {
			tx_packet->len = CDC_TX_SIZE;
			usb_cdc_transmit_flush_timer = 0;
			usb_tx(CDC_TX_ENDPOINT, tx_packet);
			tx_packet = NULL;
		}
	}
	tx_noautoflush = 0;
	return 0;
#endif
#if 0
	const uint8_t *p = (const uint8_t *)buffer;
	int r;

	while (size) {
		r = usb_serial_putchar(*p++);
		if (r < 0) return -1;
		size--;
	}
	return 0;
#endif
}

void usb_serial_flush_output(void)
{
	if (!usb_configuration) return;
	//serial_print("usb_serial_flush_output\n");
	if (tx_packet && tx_packet->index > 0) {
		usb_cdc_transmit_flush_timer = 0;
		tx_packet->len = tx_packet->index;
		usb_tx(CDC_TX_ENDPOINT, tx_packet);
		tx_packet = NULL;
	}
	// while (usb_tx_byte_count(CDC_TX_ENDPOINT) > 0) ; // wait
}

void usb_serial_flush_callback(void)
{
	if (tx_noautoflush) return;
	//serial_print("usb_flush_callback \n");
	tx_packet->len = tx_packet->index;
	usb_tx(CDC_TX_ENDPOINT, tx_packet);
	tx_packet = NULL;
	//serial_print("usb_flush_callback end\n");
}









#endif // CDC_STATUS_INTERFACE && CDC_DATA_INTERFACE
