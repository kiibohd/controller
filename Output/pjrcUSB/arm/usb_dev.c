/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander (2013-2014)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// ----- Includes -----

// Project Includes
#include <Lib/OutputLib.h>
#include <print.h>

// Local Includes
#include "usb_dev.h"
#include "usb_mem.h"



// ----- Defines -----

// DEBUG Mode
// XXX - Only use when using usbMuxUart Module
// Delay causes issues initializing more than 1 hid device (i.e. NKRO keyboard)
//#define UART_DEBUG 1
// Debug Unknown USB requests, usually what you want to debug USB issues
//#define UART_DEBUG_UNKNOWN 1


#define TX_STATE_BOTH_FREE_EVEN_FIRST   0
#define TX_STATE_BOTH_FREE_ODD_FIRST    1
#define TX_STATE_EVEN_FREE              2
#define TX_STATE_ODD_FREE               3
#define TX_STATE_NONE_FREE_EVEN_FIRST   4
#define TX_STATE_NONE_FREE_ODD_FIRST    5

#define BDT_OWN         0x80
#define BDT_DATA1       0x40
#define BDT_DATA0       0x00
#define BDT_DTS         0x08
#define BDT_STALL       0x04

#define TX    1
#define RX    0
#define ODD   1
#define EVEN  0
#define DATA0 0
#define DATA1 1


#define GET_STATUS              0
#define CLEAR_FEATURE           1
#define SET_FEATURE             3
#define SET_ADDRESS             5
#define GET_DESCRIPTOR          6
#define SET_DESCRIPTOR          7
#define GET_CONFIGURATION       8
#define SET_CONFIGURATION       9
#define GET_INTERFACE           10
#define SET_INTERFACE           11
#define SYNCH_FRAME             12

#define TX_STATE_BOTH_FREE_EVEN_FIRST   0
#define TX_STATE_BOTH_FREE_ODD_FIRST    1
#define TX_STATE_EVEN_FREE              2
#define TX_STATE_ODD_FREE               3
#define TX_STATE_NONE_FREE              4





// ----- Macros -----

#define BDT_PID(n)      (((n) >> 2) & 15)

#define BDT_DESC(count, data)   (BDT_OWN | BDT_DTS \
				| ((data) ? BDT_DATA1 : BDT_DATA0) \
				| ((count) << 16))

#define index(endpoint, tx, odd) (((endpoint) << 2) | ((tx) << 1) | (odd))
#define stat2bufferdescriptor(stat) (table + ((stat) >> 2))



// ----- Structs -----

// buffer descriptor table

typedef struct {
	uint32_t desc;
	void * addr;
} bdt_t;

static union {
	struct {
		union {
			struct {
				uint8_t bmRequestType;
				uint8_t bRequest;
			};
			uint16_t wRequestAndType;
		};
		uint16_t wValue;
		uint16_t wIndex;
		uint16_t wLength;
	};
	struct {
		uint32_t word1;
		uint32_t word2;
	};
} setup;



// ----- Variables -----

__attribute__ ((section(".usbdescriptortable"), used))
static bdt_t table[ (NUM_ENDPOINTS + 1) * 4 ];

static usb_packet_t *rx_first  [ NUM_ENDPOINTS ];
static usb_packet_t *rx_last   [ NUM_ENDPOINTS ];
static usb_packet_t *tx_first  [ NUM_ENDPOINTS ];
static usb_packet_t *tx_last   [ NUM_ENDPOINTS ];
uint16_t usb_rx_byte_count_data[ NUM_ENDPOINTS ];

static uint8_t tx_state[NUM_ENDPOINTS];

// SETUP always uses a DATA0 PID for the data field of the SETUP transaction.
// transactions in the data phase start with DATA1 and toggle (figure 8-12, USB1.1)
// Status stage uses a DATA1 PID.

static uint8_t ep0_rx0_buf[EP0_SIZE] __attribute__ ((aligned (4)));
static uint8_t ep0_rx1_buf[EP0_SIZE] __attribute__ ((aligned (4)));
static const uint8_t *ep0_tx_ptr = NULL;
static uint16_t ep0_tx_len;
static uint8_t ep0_tx_bdt_bank = 0;
static uint8_t ep0_tx_data_toggle = 0;
uint8_t usb_rx_memory_needed = 0;

volatile uint8_t usb_configuration = 0;
volatile uint8_t usb_reboot_timer = 0;

static uint8_t reply_buffer[8];



// ----- Functions -----

static void endpoint0_stall()
{
	USB0_ENDPT0 = USB_ENDPT_EPSTALL | USB_ENDPT_EPRXEN | USB_ENDPT_EPTXEN | USB_ENDPT_EPHSHK;
}

static void endpoint0_transmit( const void *data, uint32_t len )
{
	table[index(0, TX, ep0_tx_bdt_bank)].addr = (void *)data;
	table[index(0, TX, ep0_tx_bdt_bank)].desc = BDT_DESC(len, ep0_tx_data_toggle);
	ep0_tx_data_toggle ^= 1;
	ep0_tx_bdt_bank ^= 1;
}

static void usb_setup()
{
	const uint8_t *data = NULL;
	uint32_t datalen = 0;
	const usb_descriptor_list_t *list;
	uint32_t size;
	volatile uint8_t *reg;
	uint8_t epconf;
	const uint8_t *cfg;
	int i;

	switch ( setup.wRequestAndType )
	{
	case 0x0500: // SET_ADDRESS
		break;
	case 0x0900: // SET_CONFIGURATION
		#ifdef UART_DEBUG
		print("CONFIGURE - ");
		#endif
		usb_configuration = setup.wValue;
		reg = &USB0_ENDPT1;
		cfg = usb_endpoint_config_table;
		// clear all BDT entries, free any allocated memory...
		for ( i = 4; i < ( NUM_ENDPOINTS + 1) * 4; i++ )
		{
			if ( table[i].desc & BDT_OWN )
			{
				usb_free( (usb_packet_t *)((uint8_t *)(table[ i ].addr) - 8) );
			}
		}
		// free all queued packets
		for ( i = 0; i < NUM_ENDPOINTS; i++ )
		{
			usb_packet_t *p, *n;
			p = rx_first[i];
			while ( p )
			{
				n = p->next;
				usb_free(p);
				p = n;
			}
			rx_first[ i ] = NULL;
			rx_last[ i ] = NULL;
			p = tx_first[i];
			while (p)
			{
				n = p->next;
				usb_free(p);
				p = n;
			}
			tx_first[ i ] = NULL;
			tx_last[ i ] = NULL;
			usb_rx_byte_count_data[i] = 0;

			switch ( tx_state[ i ] )
			{
			case TX_STATE_EVEN_FREE:
			case TX_STATE_NONE_FREE_EVEN_FIRST:
				tx_state[ i ] = TX_STATE_BOTH_FREE_EVEN_FIRST;
				break;
			case TX_STATE_ODD_FREE:
			case TX_STATE_NONE_FREE_ODD_FIRST:
				tx_state[ i ] = TX_STATE_BOTH_FREE_ODD_FIRST;
				break;
			default:
				break;
			}
		}
		usb_rx_memory_needed = 0;
		for ( i = 1; i <= NUM_ENDPOINTS; i++ )
		{
			epconf = *cfg++;
			*reg = epconf;
			reg += 4;
			if ( epconf & USB_ENDPT_EPRXEN )
			{
				usb_packet_t *p;
				p = usb_malloc();
				if ( p )
				{
					table[ index( i, RX, EVEN ) ].addr = p->buf;
					table[ index( i, RX, EVEN ) ].desc = BDT_DESC( 64, 0 );
				}
				else
				{
					table[ index( i, RX, EVEN ) ].desc = 0;
					usb_rx_memory_needed++;
				}
				p = usb_malloc();
				if ( p )
				{
					table[ index( i, RX, ODD ) ].addr = p->buf;
					table[ index( i, RX, ODD ) ].desc = BDT_DESC( 64, 1 );
				}
				else
				{
					table[ index( i, RX, ODD ) ].desc = 0;
					usb_rx_memory_needed++;
				}
			}
			table[ index( i, TX, EVEN ) ].desc = 0;
			table[ index( i, TX, ODD ) ].desc = 0;
		}
		break;
	case 0x0880: // GET_CONFIGURATION
		reply_buffer[0] = usb_configuration;
		datalen = 1;
		data = reply_buffer;
		break;
	case 0x0080: // GET_STATUS (device)
		reply_buffer[0] = 0;
		reply_buffer[1] = 0;
		datalen = 2;
		data = reply_buffer;
		break;
	case 0x0082: // GET_STATUS (endpoint)
		if ( setup.wIndex > NUM_ENDPOINTS )
		{
			// TODO: do we need to handle IN vs OUT here?
			endpoint0_stall();
			return;
		}
		reply_buffer[0] = 0;
		reply_buffer[1] = 0;
		if ( *(uint8_t *)(&USB0_ENDPT0 + setup.wIndex * 4) & 0x02 )
			reply_buffer[0] = 1;
		data = reply_buffer;
		datalen = 2;
		break;
	case 0x0100: // CLEAR_FEATURE (device)
	case 0x0101: // CLEAR_FEATURE (interface)
		// TODO: Currently ignoring, perhaps useful? -HaaTa
		endpoint0_stall();
		return;
	case 0x0102: // CLEAR_FEATURE (interface)
		i = setup.wIndex & 0x7F;
		if ( i > NUM_ENDPOINTS || setup.wValue != 0 )
		{
			endpoint0_stall();
			return;
		}
		//(*(uint8_t *)(&USB0_ENDPT0 + setup.wIndex * 4)) &= ~0x02;
		// TODO: do we need to clear the data toggle here?
		//break;

		// FIXME: Clearing causes keyboard to freeze, likely an invalid clear
		// XXX: Ignoring seems to work, though this may not be the ideal behaviour -HaaTa
		endpoint0_stall();
		return;
	case 0x0300: // SET_FEATURE (device)
	case 0x0301: // SET_FEATURE (interface)
		// TODO: Currently ignoring, perhaps useful? -HaaTa
		endpoint0_stall();
		return;
	case 0x0302: // SET_FEATURE (endpoint)
		i = setup.wIndex & 0x7F;
		if ( i > NUM_ENDPOINTS || setup.wValue != 0 )
		{
			// TODO: do we need to handle IN vs OUT here?
			endpoint0_stall();
			return;
		}
		(*(uint8_t *)(&USB0_ENDPT0 + setup.wIndex * 4)) |= 0x02;
		// TODO: do we need to clear the data toggle here?
		break;
	case 0x0680: // GET_DESCRIPTOR
	case 0x0681:
		#ifdef UART_DEBUG
		print("desc:");
		printHex( setup.wValue );
		print( NL );
		#endif
		for ( list = usb_descriptor_list; 1; list++ )
		{
			if ( list->addr == NULL )
				break;
			if ( setup.wValue == list->wValue && setup.wIndex == list->wIndex )
			{
				data = list->addr;
				if ( (setup.wValue >> 8) == 3 )
				{
					// for string descriptors, use the descriptor's
					// length field, allowing runtime configured
					// length.
					datalen = *(list->addr);
				}
				else
				{
					datalen = list->length;
				}
				#if UART_DEBUG
				print("Desc found, ");
				printHex32( (uint32_t)data );
				print(",");
				printHex( datalen );
				print(",");
				printHex_op( data[0], 2 );
				printHex_op( data[1], 2 );
				printHex_op( data[2], 2 );
				printHex_op( data[3], 2 );
				printHex_op( data[4], 2 );
				printHex_op( data[5], 2 );
				print( NL );
				#endif
				goto send;
			}
		}
		#ifdef UART_DEBUG
		print( "desc: not found" NL );
		#endif
		endpoint0_stall();
		return;

	case 0x2221: // CDC_SET_CONTROL_LINE_STATE
		usb_cdc_line_rtsdtr = setup.wValue;
		//serial_print("set control line state\n");
		endpoint0_stall();
		return;

	case 0x21A1: // CDC_GET_LINE_CODING
		data = (uint8_t*)usb_cdc_line_coding;
		datalen = sizeof( usb_cdc_line_coding );
		goto send;

	case 0x2021: // CDC_SET_LINE_CODING
		// XXX Needed?
		//serial_print("set coding, waiting...\n");
		endpoint0_stall();
		return; // Cannot stall here (causes issues)

	case 0x0921: // HID SET_REPORT
		#ifdef UART_DEBUG
		print("SET_REPORT - ");
		printHex( setup.wValue );
		print(" - ");
		printHex( setup.wValue & 0xFF );
		print( NL );
		#endif
		USBKeys_LEDs = setup.wValue & 0xFF;
		endpoint0_stall();
		return;

	case 0x01A1: // HID GET_REPORT
		#ifdef UART_DEBUG
		print("GET_REPORT - ");
		printHex( USBKeys_LEDs );
		print(NL);
		#endif
		data = (uint8_t*)&USBKeys_LEDs;
		datalen = 1;
		goto send;

	case 0x0A21: // HID SET_IDLE
		#ifdef UART_DEBUG
		print("SET_IDLE - ");
		printHex( setup.wValue );
		print(NL);
		#endif
		USBKeys_Idle_Config = (setup.wValue >> 8);
		USBKeys_Idle_Count = 0;
		endpoint0_stall();
		return;

	case 0x0B21: // HID SET_PROTOCOL
		#ifdef UART_DEBUG
		print("SET_PROTOCOL - ");
		printHex( setup.wValue );
		print(" - ");
		printHex( setup.wValue & 0xFF );
		print(NL);
		#endif
		USBKeys_Protocol = setup.wValue & 0xFF; // 0 - Boot Mode, 1 - NKRO Mode
		endpoint0_stall();
		return;

	// case 0xC940:
	default:
		#ifdef UART_DEBUG_UNKNOWN
		print("UNKNOWN");
		#endif
		endpoint0_stall();
		return;
	}

send:
	#ifdef UART_DEBUG
	print("setup send ");
	printHex32((uint32_t)data);
	print(",");
	printHex(datalen);
	print(NL);
	#endif

	if ( datalen > setup.wLength )
		datalen = setup.wLength;

	size = datalen;
	if ( size > EP0_SIZE )
		size = EP0_SIZE;

	endpoint0_transmit(data, size);
	data += size;
	datalen -= size;

	// See if transmit has finished
	if ( datalen == 0 && size < EP0_SIZE )
		return;

	size = datalen;
	if ( size > EP0_SIZE )
		size = EP0_SIZE;
	endpoint0_transmit(data, size);
	data += size;
	datalen -= size;

	// See if transmit has finished
	if ( datalen == 0 && size < EP0_SIZE )
		return;

	// Save rest of transfer for later? XXX
	ep0_tx_ptr = data;
	ep0_tx_len = datalen;
}


//A bulk endpoint's toggle sequence is initialized to DATA0 when the endpoint
//experiences any configuration event (configuration events are explained in
//Sections 9.1.1.5 and 9.4.5).

//Configuring a device or changing an alternate setting causes all of the status
//and configuration values associated with endpoints in the affected interfaces
//to be set to their default values. This includes setting the data toggle of
//any endpoint using data toggles to the value DATA0.

//For endpoints using data toggle, regardless of whether an endpoint has the
//Halt feature set, a ClearFeature(ENDPOINT_HALT) request always results in the
//data toggle being reinitialized to DATA0.

static void usb_control( uint32_t stat )
{
	#ifdef UART_DEBUG
	print("CONTROL - ");
	#endif
	bdt_t *b;
	uint32_t pid, size;
	uint8_t *buf;
	const uint8_t *data;

	b = stat2bufferdescriptor( stat );
	pid = BDT_PID( b->desc );
	buf = b->addr;
	#ifdef UART_DEBUG
	print("pid:");
	printHex(pid);
	print(", count:");
	printHex32(b->desc);
	print(" - ");
	#endif

	switch (pid)
	{
	case 0x0D: // Setup received from host
		//serial_print("PID=Setup\n");
		//if (count != 8) ; // panic?
		// grab the 8 byte setup info
		setup.word1 = *(uint32_t *)(buf);
		setup.word2 = *(uint32_t *)(buf + 4);

		// give the buffer back
		b->desc = BDT_DESC( EP0_SIZE, DATA1 );
		//table[index(0, RX, EVEN)].desc = BDT_DESC(EP0_SIZE, 1);
		//table[index(0, RX, ODD)].desc = BDT_DESC(EP0_SIZE, 1);

		// clear any leftover pending IN transactions
		ep0_tx_ptr = NULL;
		if ( ep0_tx_data_toggle )
		{
		}
		//if (table[index(0, TX, EVEN)].desc & 0x80) {
			//serial_print("leftover tx even\n");
		//}
		//if (table[index(0, TX, ODD)].desc & 0x80) {
			//serial_print("leftover tx odd\n");
		//}
		table[index(0, TX, EVEN)].desc = 0;
		table[index(0, TX, ODD)].desc = 0;
		// first IN after Setup is always DATA1
		ep0_tx_data_toggle = 1;

		#ifdef UART_DEBUG_UNKNOWN
		print("bmRequestType:");
		printHex(setup.bmRequestType);
		print(", bRequest:");
		printHex(setup.bRequest);
		print(", wValue:");
		printHex(setup.wValue);
		print(", wIndex:");
		printHex(setup.wIndex);
		print(", len:");
		printHex(setup.wLength);
		print(NL);
		#endif
		// actually "do" the setup request
		usb_setup();
		// unfreeze the USB, now that we're ready
		USB0_CTL = USB_CTL_USBENSOFEN; // clear TXSUSPENDTOKENBUSY bit
		break;
	case 0x01:  // OUT transaction received from host
	case 0x02:
		#ifdef UART_DEBUG
		print("PID=OUT"NL);
		#endif
		// CDC Interface
		if ( setup.wRequestAndType == 0x2021 /*CDC_SET_LINE_CODING*/ )
		{
			int i;
			uint8_t *dst = (uint8_t *)usb_cdc_line_coding;
			//serial_print("set line coding ");
			for ( i = 0; i < 7; i++ )
			{
				//serial_phex(*buf);
				*dst++ = *buf++;
			}
			//serial_phex32(usb_cdc_line_coding[0]);
			//serial_print("\n");
			if ( usb_cdc_line_coding[0] == 134 )
				usb_reboot_timer = 15;
			endpoint0_transmit( NULL, 0 );
		}

		// Keyboard Interface
		if ( setup.word1 == 0x02000921 && setup.word2 == ( (1<<16) | KEYBOARD_INTERFACE ) )
		{
			USBKeys_LEDs = buf[0];
			endpoint0_transmit( NULL, 0 );
		}
		// NKRO Keyboard Interface
		if ( setup.word1 == 0x02000921 && setup.word2 == ( (1<<16) | NKRO_KEYBOARD_INTERFACE ) )
		{
			USBKeys_LEDs = buf[0];
			endpoint0_transmit( NULL, 0 );
		}

		// give the buffer back
		b->desc = BDT_DESC( EP0_SIZE, DATA1 );
		break;

	case 0x09: // IN transaction completed to host
		#ifdef UART_DEBUG
		print("PID=IN:");
		printHex(stat);
		print(NL);
		#endif

		// send remaining data, if any...
		data = ep0_tx_ptr;
		if ( data )
		{
			size = ep0_tx_len;
			if (size > EP0_SIZE) size = EP0_SIZE;
			endpoint0_transmit(data, size);
			data += size;
			ep0_tx_len -= size;
			ep0_tx_ptr = (ep0_tx_len > 0 || size == EP0_SIZE) ? data : NULL;
		}

		if ( setup.bRequest == 5 && setup.bmRequestType == 0 )
		{
			setup.bRequest = 0;
			#ifdef UART_DEBUG
			print("set address: ");
			printHex(setup.wValue);
			print(NL);
			#endif
			USB0_ADDR = setup.wValue;
		}

		break;
	default:
		#ifdef UART_DEBUG
		print("PID=unknown:");
		printHex(pid);
		print(NL);
		#endif
		break;
	}
	USB0_CTL = USB_CTL_USBENSOFEN; // clear TXSUSPENDTOKENBUSY bit
}

usb_packet_t *usb_rx( uint32_t endpoint )
{
	//print("USB RX");
	usb_packet_t *ret;
	endpoint--;
	if ( endpoint >= NUM_ENDPOINTS )
		return NULL;
	__disable_irq();
	ret = rx_first[endpoint];
	if ( ret )
		rx_first[ endpoint ] = ret->next;
	usb_rx_byte_count_data[ endpoint ] -= ret->len;
	__enable_irq();
	//serial_print("rx, epidx=");
	//serial_phex(endpoint);
	//serial_print(", packet=");
	//serial_phex32(ret);
	//serial_print("\n");
	return ret;
}

static uint32_t usb_queue_byte_count( const usb_packet_t *p )
{
	uint32_t count=0;

	__disable_irq();
	for ( ; p; p = p->next )
	{
		count += p->len;
	}
	__enable_irq();
	return count;
}

uint32_t usb_tx_byte_count( uint32_t endpoint )
{
	endpoint--;
	if ( endpoint >= NUM_ENDPOINTS )
		return 0;
	return usb_queue_byte_count( tx_first[ endpoint ] );
}

uint32_t usb_tx_packet_count( uint32_t endpoint )
{
	const usb_packet_t *p;
	uint32_t count=0;

	endpoint--;
	if ( endpoint >= NUM_ENDPOINTS )
		return 0;
	__disable_irq();
	for ( p = tx_first[ endpoint ]; p; p = p->next )
		count++;
	__enable_irq();
	return count;
}


// Called from usb_free, but only when usb_rx_memory_needed > 0, indicating
// receive endpoints are starving for memory.  The intention is to give
// endpoints needing receive memory priority over the user's code, which is
// likely calling usb_malloc to obtain memory for transmitting.  When the
// user is creating data very quickly, their consumption could starve reception
// without this prioritization.  The packet buffer (input) is assigned to the
// first endpoint needing memory.
//
void usb_rx_memory( usb_packet_t *packet )
{
	//print("USB RX MEMORY");
	unsigned int i;
	const uint8_t *cfg;

	cfg = usb_endpoint_config_table;
	//serial_print("rx_mem:");
	__disable_irq();
	for ( i = 1; i <= NUM_ENDPOINTS; i++ )
	{
		if ( *cfg++ & USB_ENDPT_EPRXEN )
		{
			if ( table[ index( i, RX, EVEN ) ].desc == 0 )
			{
				table[ index( i, RX, EVEN ) ].addr = packet->buf;
				table[ index( i, RX, EVEN ) ].desc = BDT_DESC( 64, 0 );
				usb_rx_memory_needed--;
				__enable_irq();
				//serial_phex(i);
				//serial_print(",even\n");
				return;
			}
			if ( table[ index( i, RX, ODD ) ].desc == 0 )
			{
				table[ index( i, RX, ODD ) ].addr = packet->buf;
				table[ index( i, RX, ODD ) ].desc = BDT_DESC( 64, 1 );
				usb_rx_memory_needed--;
				__enable_irq();
				//serial_phex(i);
				//serial_print(",odd\n");
				return;
			}
		}
	}
	__enable_irq();
	// we should never reach this point.  If we get here, it means
	// usb_rx_memory_needed was set greater than zero, but no memory
	// was actually needed.
	usb_rx_memory_needed = 0;
	usb_free( packet );
	return;
}

//#define index(endpoint, tx, odd) (((endpoint) << 2) | ((tx) << 1) | (odd))
//#define stat2bufferdescriptor(stat) (table + ((stat) >> 2))

void usb_tx( uint32_t endpoint, usb_packet_t *packet )
{
	bdt_t *b = &table[ index( endpoint, TX, EVEN ) ];
	uint8_t next;

	endpoint--;
	if ( endpoint >= NUM_ENDPOINTS )
		return;
	__disable_irq();
	//serial_print("txstate=");
	//serial_phex(tx_state[ endpoint ]);
	//serial_print("\n");
	switch ( tx_state[ endpoint ] )
	{
	case TX_STATE_BOTH_FREE_EVEN_FIRST:
		next = TX_STATE_ODD_FREE;
		break;
	case TX_STATE_BOTH_FREE_ODD_FIRST:
		b++;
		next = TX_STATE_EVEN_FREE;
		break;
	case TX_STATE_EVEN_FREE:
		next = TX_STATE_NONE_FREE_ODD_FIRST;
		break;
	case TX_STATE_ODD_FREE:
		b++;
		next = TX_STATE_NONE_FREE_EVEN_FIRST;
		break;
	default:
		if (tx_first[ endpoint ] == NULL)
		{
			tx_first[ endpoint ] = packet;
		}
		else
		{
			tx_last[ endpoint ]->next = packet;
		}
		tx_last[ endpoint ] = packet;
		__enable_irq();
		return;
	}

	tx_state[ endpoint ] = next;
	b->addr = packet->buf;
	b->desc = BDT_DESC( packet->len, ((uint32_t)b & 8) ? DATA1 : DATA0 );
	__enable_irq();
}


void usb_device_reload()
{
// MCHCK
#if defined(_mk20dx128vlf5_)

	// MCHCK Kiibohd Variant
	// Check to see if PTA3 (has a pull-up) is connected to GND (usually via jumper)
	// Only allow reload if the jumper is present (security)
	GPIOA_PDDR &= ~(1<<3); // Input
	PORTA_PCR3 = PORT_PCR_PFE | PORT_PCR_MUX(1); // Internal pull-up

	// Check for jumper
	if ( GPIOA_PDIR & (1<<3) )
	{
		print( NL );
		warn_print("Security jumper not present, cancelling firmware reload...");
		info_msg("Replace jumper on middle 2 pins, or manually press the firmware reload button.");
	}
	else
	{
		// Copies variable into the VBAT register, must be identical to the variable in the bootloader to jump to the bootloader flash mode
		for ( int pos = 0; pos < sizeof(sys_reset_to_loader_magic); pos++ )
			(&VBAT)[ pos ] = sys_reset_to_loader_magic[ pos ];
		SOFTWARE_RESET();
	}

// Teensy 3.0 and 3.1
#else
	asm volatile("bkpt");
#endif
}


void usb_isr()
{
	uint8_t status, stat, t;

	//serial_print("isr");
	//status = USB0_ISTAT;
	//serial_phex(status);
	//serial_print("\n");
restart:
	status = USB0_ISTAT;
	/*
	print("USB ISR STATUS: ");
	printHex( status );
	print( NL );
	*/

	if ( (status & USB_INTEN_SOFTOKEN /* 04 */ ) )
	{
		if ( usb_configuration )
		{
			t = usb_reboot_timer;
			if ( t )
			{
				usb_reboot_timer = --t;
				if ( !t )
					usb_device_reload();
			}

			// CDC Interface
			t = usb_cdc_transmit_flush_timer;
			if ( t )
			{
				usb_cdc_transmit_flush_timer = --t;
				if ( t == 0 )
					usb_serial_flush_callback();
			}

		}
		USB0_ISTAT = USB_INTEN_SOFTOKEN;
	}

	if ( (status & USB_ISTAT_TOKDNE /* 08 */ ) )
	{
		uint8_t endpoint;
		stat = USB0_STAT;
		//serial_print("token: ep=");
		//serial_phex(stat >> 4);
		//serial_print(stat & 0x08 ? ",tx" : ",rx");
		//serial_print(stat & 0x04 ? ",odd\n" : ",even\n");
		endpoint = stat >> 4;
		if ( endpoint == 0 )
		{
			usb_control( stat );
		}
		else
		{
			bdt_t *b = stat2bufferdescriptor(stat);
			usb_packet_t *packet = (usb_packet_t *)((uint8_t *)(b->addr) - 8);
#if 0
			serial_print("ep:");
			serial_phex(endpoint);
			serial_print(", pid:");
			serial_phex(BDT_PID(b->desc));
			serial_print(((uint32_t)b & 8) ? ", odd" : ", even");
			serial_print(", count:");
			serial_phex(b->desc >> 16);
			serial_print("\n");
#endif
			endpoint--;     // endpoint is index to zero-based arrays

			if ( stat & 0x08 )
			{ // transmit
				usb_free( packet );
				packet = tx_first[ endpoint ];
				if ( packet )
				{
					//serial_print("tx packet\n");
					tx_first[endpoint] = packet->next;
					b->addr = packet->buf;
					switch ( tx_state[ endpoint ] )
					{
					case TX_STATE_BOTH_FREE_EVEN_FIRST:
						tx_state[ endpoint ] = TX_STATE_ODD_FREE;
						break;
					case TX_STATE_BOTH_FREE_ODD_FIRST:
						tx_state[ endpoint ] = TX_STATE_EVEN_FREE;
						break;
					case TX_STATE_EVEN_FREE:
						tx_state[ endpoint ] = TX_STATE_NONE_FREE_ODD_FIRST;
						break;
					case TX_STATE_ODD_FREE:
						tx_state[ endpoint ] = TX_STATE_NONE_FREE_EVEN_FIRST;
						break;
					default:
						break;
					}
					b->desc = BDT_DESC( packet->len, ((uint32_t)b & 8) ? DATA1 : DATA0 );
				} else {
					//serial_print("tx no packet\n");
					switch ( tx_state[ endpoint ] )
					{
					case TX_STATE_BOTH_FREE_EVEN_FIRST:
					case TX_STATE_BOTH_FREE_ODD_FIRST:
						break;
					case TX_STATE_EVEN_FREE:
						tx_state[ endpoint ] = TX_STATE_BOTH_FREE_EVEN_FIRST;
						break;
					case TX_STATE_ODD_FREE:
						tx_state[ endpoint ] = TX_STATE_BOTH_FREE_ODD_FIRST;
						break;
					default:
						tx_state[ endpoint ] = ((uint32_t)b & 8)
						  ? TX_STATE_ODD_FREE
						  : TX_STATE_EVEN_FREE;
						break;
					}
				}
			}
			else
			{ // receive
				packet->len = b->desc >> 16;
				if ( packet->len > 0 )
				{
					packet->index = 0;
					packet->next = NULL;
					if ( rx_first[ endpoint ] == NULL )
					{
						//serial_print("rx 1st, epidx=");
						//serial_phex(endpoint);
						//serial_print(", packet=");
						//serial_phex32((uint32_t)packet);
						//serial_print("\n");
						rx_first[ endpoint ] = packet;
					}
					else
					{
						//serial_print("rx Nth, epidx=");
						//serial_phex(endpoint);
						//serial_print(", packet=");
						//serial_phex32((uint32_t)packet);
						//serial_print("\n");
						rx_last[ endpoint ]->next = packet;
					}
					rx_last[ endpoint ] = packet;
					usb_rx_byte_count_data[ endpoint ] += packet->len;
					// TODO: implement a per-endpoint maximum # of allocated packets
					// so a flood of incoming data on 1 endpoint doesn't starve
					// the others if the user isn't reading it regularly
					packet = usb_malloc();
					if ( packet )
					{
						b->addr = packet->buf;
						b->desc = BDT_DESC( 64, ((uint32_t)b & 8) ? DATA1 : DATA0 );
					}
					else
					{
						//serial_print("starving ");
						//serial_phex(endpoint + 1);
						//serial_print(((uint32_t)b & 8) ? ",odd\n" : ",even\n");
						b->desc = 0;
						usb_rx_memory_needed++;
					}
				}
				else
				{
					b->desc = BDT_DESC( 64, ((uint32_t)b & 8) ? DATA1 : DATA0 );
				}
			}




		}
		USB0_ISTAT = USB_ISTAT_TOKDNE;
		goto restart;
	}


	if ( status & USB_ISTAT_USBRST /* 01 */ )
	{
		//serial_print("reset\n");

		// initialize BDT toggle bits
		USB0_CTL = USB_CTL_ODDRST;
		ep0_tx_bdt_bank = 0;

		// set up buffers to receive Setup and OUT packets
		table[index( 0, RX, EVEN ) ].desc = BDT_DESC( EP0_SIZE, 0 );
		table[index( 0, RX, EVEN ) ].addr = ep0_rx0_buf;
		table[index( 0, RX, ODD ) ].desc = BDT_DESC( EP0_SIZE, 0 );
		table[index( 0, RX, ODD ) ].addr = ep0_rx1_buf;
		table[index( 0, TX, EVEN ) ].desc = 0;
		table[index( 0, TX, ODD ) ].desc = 0;

		// activate endpoint 0
		USB0_ENDPT0 = USB_ENDPT_EPRXEN | USB_ENDPT_EPTXEN | USB_ENDPT_EPHSHK;

		// clear all ending interrupts
		USB0_ERRSTAT = 0xFF;
		USB0_ISTAT = 0xFF;

		// set the address to zero during enumeration
		USB0_ADDR = 0;

		// enable other interrupts
		USB0_ERREN = 0xFF;
		USB0_INTEN = USB_INTEN_TOKDNEEN |
			USB_INTEN_SOFTOKEN |
			USB_INTEN_STALLEN |
			USB_INTEN_ERROREN |
			USB_INTEN_USBRSTEN |
			USB_INTEN_SLEEPEN;

		// is this necessary?
		USB0_CTL = USB_CTL_USBENSOFEN;
		return;
	}


	if ( (status & USB_ISTAT_STALL /* 80 */ ) )
	{
		//serial_print("stall:\n");
		USB0_ENDPT0 = USB_ENDPT_EPRXEN | USB_ENDPT_EPTXEN | USB_ENDPT_EPHSHK;
		USB0_ISTAT = USB_ISTAT_STALL;
	}
	if ( (status & USB_ISTAT_ERROR /* 02 */ ) )
	{
		uint8_t err = USB0_ERRSTAT;
		USB0_ERRSTAT = err;
		//serial_print("err:");
		//serial_phex(err);
		//serial_print("\n");
		USB0_ISTAT = USB_ISTAT_ERROR;
	}

	if ( (status & USB_ISTAT_SLEEP /* 10 */ ) )
	{
		//serial_print("sleep\n");
		USB0_ISTAT = USB_ISTAT_SLEEP;
	}
}



uint8_t usb_init()
{
	#ifdef UART_DEBUG
	print("USB INIT"NL);
	#endif

	// If no USB cable is attached, do not initialize usb
	// XXX Test -HaaTa
	//if ( USB0_OTGISTAT & USB_OTGSTAT_ID )
	//      return 0;

	// Clear out endpoints table
	for ( int i = 0; i <= NUM_ENDPOINTS * 4; i++ )
	{
		table[i].desc = 0;
		table[i].addr = 0;
	}

	// this basically follows the flowchart in the Kinetis
	// Quick Reference User Guide, Rev. 1, 03/2012, page 141

	// assume 48 MHz clock already running
	// SIM - enable clock
	SIM_SCGC4 |= SIM_SCGC4_USBOTG;

	// reset USB module
	USB0_USBTRC0 = USB_USBTRC_USBRESET;
	while ( (USB0_USBTRC0 & USB_USBTRC_USBRESET) != 0 ); // wait for reset to end

	// set desc table base addr
	USB0_BDTPAGE1 = ((uint32_t)table) >> 8;
	USB0_BDTPAGE2 = ((uint32_t)table) >> 16;
	USB0_BDTPAGE3 = ((uint32_t)table) >> 24;

	// clear all ISR flags
	USB0_ISTAT = 0xFF;
	USB0_ERRSTAT = 0xFF;
	USB0_OTGISTAT = 0xFF;

	USB0_USBTRC0 |= 0x40; // undocumented bit

	// enable USB
	USB0_CTL = USB_CTL_USBENSOFEN;
	USB0_USBCTRL = 0;

	// enable reset interrupt
	USB0_INTEN = USB_INTEN_USBRSTEN;

	// enable interrupt in NVIC...
	NVIC_SET_PRIORITY( IRQ_USBOTG, 112 );
	NVIC_ENABLE_IRQ( IRQ_USBOTG );

	// enable d+ pullup
	USB0_CONTROL = USB_CONTROL_DPPULLUPNONOTG;

	return 1;
}

// return 0 if the USB is not configured, or the configuration
// number selected by the HOST
uint8_t usb_configured()
{
	return usb_configuration;
}

