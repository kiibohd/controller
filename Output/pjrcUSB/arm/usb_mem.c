#include <Lib/USBLib.h>
#include "usb_dev.h"
#include "usb_mem.h"

#define NUM_BUF 30

__attribute__ ((section(".usbbuffers"), used))
//static unsigned char usb_buffer_memory[NUM_BUF * sizeof(usb_packet_t)];
unsigned char usb_buffer_memory[NUM_BUF * sizeof(usb_packet_t)];

static uint32_t usb_buffer_available = 0xFFFFFFFF;

// use bitmask and CLZ instruction to implement fast free list
// http://www.archivum.info/gnu.gcc.help/2006-08/00148/Re-GCC-Inline-Assembly.html
// http://gcc.gnu.org/ml/gcc/2012-06/msg00015.html
// __builtin_clz()

usb_packet_t * usb_malloc(void)
{
	unsigned int n, avail;
	uint8_t *p;

	__disable_irq();
	avail = usb_buffer_available;
	n = __builtin_clz(avail); // clz = count leading zeros
	if (n >= NUM_BUF) {
		__enable_irq();
		return NULL;
	}
	//serial_print("malloc:");
	//serial_phex(n);
	//serial_print("\n");

	usb_buffer_available = avail & ~(0x80000000 >> n);
	__enable_irq();
	p = usb_buffer_memory + (n * sizeof(usb_packet_t));
	//serial_print("malloc:");
	//serial_phex32((int)p);
	//serial_print("\n");
	*(uint32_t *)p = 0;
	*(uint32_t *)(p + 4) = 0;
	return (usb_packet_t *)p;
}

// for the receive endpoints to request memory
extern uint8_t usb_rx_memory_needed;
extern void usb_rx_memory(usb_packet_t *packet);

void usb_free(usb_packet_t *p)
{
	unsigned int n, mask;

	//serial_print("free:");
	n = ((uint8_t *)p - usb_buffer_memory) / sizeof(usb_packet_t);
	if (n >= NUM_BUF) return;
	//serial_phex(n);
	//serial_print("\n");

	// if any endpoints are starving for memory to receive
	// packets, give this memory to them immediately!
	if (usb_rx_memory_needed && usb_configuration) {
		//serial_print("give to rx:");
		//serial_phex32((int)p);
		//serial_print("\n");
		usb_rx_memory(p);
		return;
	}

	mask = (0x80000000 >> n);
	__disable_irq();
	usb_buffer_available |= mask;
	__enable_irq();

	//serial_print("free:");
	//serial_phex32((int)p);
	//serial_print("\n");
}

