#ifndef USBserial_h_
#define USBserial_h_

#include <inttypes.h>

// Compatibility defines from AVR
#define PROGMEM
#define PGM_P  const char *
#define PSTR(str) (str)


int usb_serial_getchar(void);
int usb_serial_peekchar(void);
int usb_serial_available(void);
void usb_serial_flush_input(void);
int usb_serial_putchar(uint8_t c);
int usb_serial_write(const void *buffer, uint32_t size);
void usb_serial_flush_output(void);
extern uint8_t usb_cdc_line_coding[7];
extern volatile uint8_t usb_cdc_line_rtsdtr;
extern volatile uint8_t usb_cdc_transmit_flush_timer;
extern volatile uint8_t usb_configuration;

#endif // USBserial_h_

