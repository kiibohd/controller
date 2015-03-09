/* Teensy Loader, Command Line Interface
 * Program and Reboot Teensy Board with HalfKay Bootloader
 * http://www.pjrc.com/teensy/loader_cli.html
 * Copyright 2008-2010, PJRC.COM, LLC
 * Modifications 2014, Jacob Alexander <haata@kiibohd.com>
 *
 * You may redistribute this program and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

/* Want to incorporate this code into a proprietary application??
 * Just email paul@pjrc.com to ask.  Usually it's not a problem,
 * but you do need to ask to use this code in any way other than
 * those permitted by the GNU General Public License, version 3  */

/* For non-root permissions on ubuntu or similar udev-based linux
 * http://www.pjrc.com/teensy/49-teensy.rules
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

void usage(void)
{
	fprintf(stderr, "Usage: teensy_loader_cli -mmcu=<MCU> [-w] [-h] [-n] [-v] <file.hex>\n");
	fprintf(stderr, "\t-w : Wait for device to appear\n");
	fprintf(stderr, "\t-r : Use hard reboot if device not online\n");
	fprintf(stderr, "\t-n : No reboot after programming\n");
	fprintf(stderr, "\t-v : Verbose output\n");
#if defined(USE_LIBUSB)
	fprintf(stderr, "\n<MCU> = atmega32u4 | at90usb162 | at90usb646 | at90usb1286 | mk20dx128 | mk20dx256\n");
#else
	fprintf(stderr, "\n<MCU> = atmega32u4 | at90usb162 | at90usb646 | at90usb1286\n");
#endif
	fprintf(stderr, "\nFor more information, please visit:\n");
	fprintf(stderr, "http://www.pjrc.com/teensy/loader_cli.html\n");
	exit(1);
}

// USB Access Functions
int teensy_open(void);
int teensy_write(void *buf, int len, double timeout);
void teensy_close(void);
int hard_reboot(void);

// Intel Hex File Functions
int read_intel_hex(const char *filename);
int ihex_bytes_within_range(int begin, int end);
void ihex_get_data(int addr, int len, unsigned char *bytes);
int memory_is_blank(int addr, int block_size);

// Misc stuff
int printf_verbose(const char *format, ...);
void delay(double seconds);
void die(const char *str, ...);
void parse_options(int argc, char **argv);

// options (from user via command line args)
int wait_for_device_to_appear = 0;
int hard_reboot_device = 0;
int reboot_after_programming = 1;
int verbose = 0;
int code_size = 0, block_size = 0;
const char *filename=NULL;


/****************************************************************/
/*                                                              */
/*                       Main Program                           */
/*                                                              */
/****************************************************************/

int main(int argc, char **argv)
{
	unsigned char buf[2048];
	int num, addr, r, write_size=block_size+2;
	int first_block=1, waited=0;

	// parse command line arguments
	parse_options(argc, argv);
	if (!filename) {
		fprintf(stderr, "Filename must be specified\n\n");
		usage();
	}
	if (!code_size) {
		fprintf(stderr, "MCU type must be specified\n\n");
		usage();
	}
	printf_verbose("Teensy Loader, Command Line, Version 2.0, Kiibohd-Mods\n");

	// read the intel hex file
	// this is done first so any error is reported before using USB
	num = read_intel_hex(filename);
	if (num < 0) die("error reading intel hex file \"%s\"", filename);
	printf_verbose("Read \"%s\": %d bytes, %.1f%% usage\n",
		filename, num, (double)num / (double)code_size * 100.0);

	// open the USB device
	while (1) {
		if (teensy_open()) break;
		if (hard_reboot_device) {
			if (!hard_reboot()) die("Unable to find rebootor\n");
			printf_verbose("Hard Reboot performed\n");
			hard_reboot_device = 0; // only hard reboot once
			wait_for_device_to_appear = 1;
		}
		if (!wait_for_device_to_appear) die("Unable to open device\n");
		if (!waited) {
			printf_verbose("Waiting for Teensy device...\n");
			printf_verbose(" (hint: press the reset button)\n");
			waited = 1;
		}
		delay(0.25);
	}
	printf_verbose("Found HalfKay Bootloader\n");

	// if we waited for the device, read the hex file again
	// perhaps it changed while we were waiting?
	if (waited) {
		num = read_intel_hex(filename);
		if (num < 0) die("error reading intel hex file \"%s\"", filename);
		printf_verbose("Read \"%s\": %d bytes, %.1f%% usage\n",
			filename, num, (double)num / (double)code_size * 100.0);
	}

	// program the data
	printf_verbose("Programming");
	fflush(stdout);
	for (addr = 0; addr < code_size; addr += block_size) {
		if (!first_block && !ihex_bytes_within_range(addr, addr + block_size - 1)) {
			// don't waste time on blocks that are unused,
			// but always do the first one to erase the chip
			continue;
		}
		if (!first_block && memory_is_blank(addr, block_size)) continue;
		printf_verbose(".");
		if (code_size < 0x10000) {
			buf[0] = addr & 255;
			buf[1] = (addr >> 8) & 255;
			ihex_get_data(addr, block_size, buf + 2);
			write_size = block_size + 2;
		} else if (block_size == 256) {
			buf[0] = (addr >> 8) & 255;
			buf[1] = (addr >> 16) & 255;
			ihex_get_data(addr, block_size, buf + 2);
			write_size = block_size + 2;
		} else if (block_size >= 1024) {
			buf[0] = addr & 255;
			buf[1] = (addr >> 8) & 255;
			buf[2] = (addr >> 16) & 255;
			memset(buf + 3, 0, 61);
			ihex_get_data(addr, block_size, buf + 64);
			write_size = block_size + 64;
		} else {
			die("Unknown code/block size\n");
		}
		r = teensy_write(buf, write_size, first_block ? 3.0 : 0.25);
		if (!r) die("error writing to Teensy\n");
		first_block = 0;
	}
	printf_verbose("\n");

	// reboot to the user's new code
	if (reboot_after_programming) {
		printf_verbose("Booting\n");
		buf[0] = 0xFF;
		buf[1] = 0xFF;
		buf[2] = 0xFF;
		memset(buf + 3, 0, sizeof(buf) - 3);
		teensy_write(buf, write_size, 0.25);
	}
	teensy_close();
	return 0;
}




/****************************************************************/
/*                                                              */
/*             USB Access - libusb (Linux, Windows & FreeBSD)   */
/*                                                              */
/****************************************************************/

#if defined(USE_LIBUSB)

#include <libusb-1.0/libusb.h>

struct libusb_device_handle *open_usb_device( int vid, int pid )
{
	libusb_device **devs;
	struct libusb_device_handle *handle = NULL;
	struct libusb_device_handle *ret = NULL;

	if ( libusb_init( NULL ) != 0 )
		fprintf( stderr, "libusb_init failed.\n" );

	size_t count = libusb_get_device_list( NULL, &devs );
	if ( count < 0 )
		fprintf( stderr, "libusb_get_device_list failed.\n" );

	for ( size_t c = 0; c < count; c++ )
	{
		struct libusb_device_descriptor desc;
		libusb_device *dev = devs[c];

		if ( libusb_get_device_descriptor( dev, &desc ) < 0 )
			fprintf( stderr, "libusb_get_device_descriptor failed.\n" );

		//printf("ID: %04x  Product: %04x\n", desc.idVendor, desc.idProduct );
		// Not correct device
		if ( desc.idVendor != vid || desc.idProduct != pid )
			continue;

		// Attempt to open the device
		if ( libusb_open( dev, &handle ) != 0 )
		{
			fprintf( stderr, "Found device but unable to open\n" );
			continue;
		}

		// Only required on Linux, other platforms will just ignore this call
		libusb_detach_kernel_driver( handle, 0 );

		// Mac OS-X - removing this call to usb_claim_interface() might allow
		// this to work, even though it is a clear misuse of the libusb API.
		// normally Apple's IOKit should be used on Mac OS-X
		// XXX Is this still valid with libusb-1.0?

		// Attempt to claim interface
		int err = libusb_claim_interface( handle, 0 );
		if ( err < 0 )
		{
			libusb_close( handle );
			fprintf( stderr, "Unable to claim interface, check USB permissions: %d\n", err );
			continue;
		}

		ret = handle;
		break;
	}

	libusb_free_device_list( devs, 1 );

	return ret;
}

static struct libusb_device_handle *libusb_teensy_handle = NULL;

int teensy_open()
{
	teensy_close();

	libusb_teensy_handle = open_usb_device( 0x16C0, 0x0478 );

	if ( libusb_teensy_handle )
		return 1;

	return 0;
}

int teensy_write( void *buf, int len, double timeout )
{
	int r;

	if ( !libusb_teensy_handle )
		return 0;

	while ( timeout > 0 ) {
		r = libusb_control_transfer( libusb_teensy_handle,
			0x21, 9, 0x0200, 0,
			(unsigned char *)buf, len,
			(int)(timeout * 1000.0) );

		if ( r >= 0 )
			return 1;

		//printf("teensy_write, r=%d\n", r);
		usleep( 10000 );
		timeout -= 0.01;  // TODO: subtract actual elapsed time
	}

	return 0;
}

void teensy_close()
{
	if ( !libusb_teensy_handle)
		return;

	libusb_release_interface( libusb_teensy_handle, 0 );
	libusb_close( libusb_teensy_handle );

	libusb_teensy_handle = NULL;
}

int hard_reboot()
{
	struct libusb_device_handle *rebootor;
	int r;

	rebootor = open_usb_device( 0x16C0, 0x0477 );

	if (!rebootor)
		return 0;

	r = libusb_control_transfer( rebootor,
		0x21, 9, 0x0200, 0,
		(unsigned char*)"reboot", 6,
		100 );

	libusb_release_interface( rebootor, 0 );
	libusb_close( rebootor );

	if (r < 0)
		return 0;

	return 1;
}

#endif



/****************************************************************/
/*                                                              */
/*             USB Access - Apple's IOKit, Mac OS-X             */
/*                                                              */
/****************************************************************/

#if defined(USE_APPLE_IOKIT)

// http://developer.apple.com/technotes/tn2007/tn2187.html
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDDevice.h>

struct usb_list_struct {
	IOHIDDeviceRef ref;
	int pid;
	int vid;
	struct usb_list_struct *next;
};

static struct usb_list_struct *usb_list=NULL;
static IOHIDManagerRef hid_manager=NULL;

void attach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
	CFTypeRef type;
	struct usb_list_struct *n, *p;
	int32_t pid, vid;

	if (!dev) return;
	type = IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDVendorIDKey));
	if (!type || CFGetTypeID(type) != CFNumberGetTypeID()) return;
	if (!CFNumberGetValue((CFNumberRef)type, kCFNumberSInt32Type, &vid)) return;
	type = IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDProductIDKey));
	if (!type || CFGetTypeID(type) != CFNumberGetTypeID()) return;
	if (!CFNumberGetValue((CFNumberRef)type, kCFNumberSInt32Type, &pid)) return;
	n = (struct usb_list_struct *)malloc(sizeof(struct usb_list_struct));
	if (!n) return;
	//printf("attach callback: vid=%04X, pid=%04X\n", vid, pid);
	n->ref = dev;
	n->vid = vid;
	n->pid = pid;
	n->next = NULL;
	if (usb_list == NULL) {
		usb_list = n;
	} else {
		for (p = usb_list; p->next; p = p->next) ;
		p->next = n;
	}
}

void detach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
	struct usb_list_struct *p, *tmp, *prev=NULL;

	p = usb_list;
	while (p) {
		if (p->ref == dev) {
			if (prev) {
				prev->next = p->next;
			} else {
				usb_list = p->next;
			}
			tmp = p;
			p = p->next;
			free(tmp);
		} else {
			prev = p;
			p = p->next;
		}
	}
}

void init_hid_manager(void)
{
	CFMutableDictionaryRef dict;
	IOReturn ret;

	if (hid_manager) return;
	hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	if (hid_manager == NULL || CFGetTypeID(hid_manager) != IOHIDManagerGetTypeID()) {
		if (hid_manager) CFRelease(hid_manager);
		printf_verbose("no HID Manager - maybe this is a pre-Leopard (10.5) system?\n");
		return;
	}
	dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (!dict) return;
	IOHIDManagerSetDeviceMatching(hid_manager, dict);
	CFRelease(dict);
	IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, attach_callback, NULL);
	IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, detach_callback, NULL);
	ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
	if (ret != kIOReturnSuccess) {
		IOHIDManagerUnscheduleFromRunLoop(hid_manager,
			CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		CFRelease(hid_manager);
		printf_verbose("Error opening HID Manager");
	}
}

static void do_run_loop(void)
{
	while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) ;
}

IOHIDDeviceRef open_usb_device(int vid, int pid)
{
	struct usb_list_struct *p;
	IOReturn ret;

	init_hid_manager();
	do_run_loop();
	for (p = usb_list; p; p = p->next) {
		if (p->vid == vid && p->pid == pid) {
			ret = IOHIDDeviceOpen(p->ref, kIOHIDOptionsTypeNone);
			if (ret == kIOReturnSuccess) return p->ref;
		}
	}
	return NULL;
}

void close_usb_device(IOHIDDeviceRef dev)
{
	struct usb_list_struct *p;

	do_run_loop();
	for (p = usb_list; p; p = p->next) {
		if (p->ref == dev) {
			IOHIDDeviceClose(dev, kIOHIDOptionsTypeNone);
			return;
		}
	}
}

static IOHIDDeviceRef iokit_teensy_reference = NULL;

int teensy_open(void)
{
	teensy_close();
	iokit_teensy_reference = open_usb_device(0x16C0, 0x0478);
	if (iokit_teensy_reference) return 1;
	return 0;
}

int teensy_write(void *buf, int len, double timeout)
{
	IOReturn ret;

	// timeouts do not work on OS-X
	// IOHIDDeviceSetReportWithCallback is not implemented
	// even though Apple documents it with a code example!
	// submitted to Apple on 22-sep-2009, problem ID 7245050
	if (!iokit_teensy_reference) return 0;
	ret = IOHIDDeviceSetReport(iokit_teensy_reference,
		kIOHIDReportTypeOutput, 0, buf, len);
	if (ret == kIOReturnSuccess) return 1;
	return 0;
}

void teensy_close(void)
{
	if (!iokit_teensy_reference) return;
	close_usb_device(iokit_teensy_reference);
	iokit_teensy_reference = NULL;
}

int hard_reboot(void)
{
	IOHIDDeviceRef rebootor;
	IOReturn ret;

	rebootor = open_usb_device(0x16C0, 0x0477);
	if (!rebootor) return 0;
	ret = IOHIDDeviceSetReport(rebootor,
		kIOHIDReportTypeOutput, 0, (uint8_t *)("reboot"), 6);
	close_usb_device(rebootor);
	if (ret == kIOReturnSuccess) return 1;
	return 0;
}

#endif



/****************************************************************/
/*                                                              */
/*              USB Access - BSD's UHID driver                  */
/*                                                              */
/****************************************************************/

#if defined(USE_UHID)

// Thanks to Todd T Fries for help getting this working on OpenBSD
// and to Chris Kuethe for the initial patch to use UHID.

#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <dev/usb/usb.h>
#ifndef USB_GET_DEVICEINFO
#include <dev/usb/usb_ioctl.h>
#endif

int open_usb_device(int vid, int pid)
{
	int r, fd;
	DIR *dir;
	struct dirent *d;
	struct usb_device_info info;
	char buf[256];

	dir = opendir("/dev");
	if (!dir) return -1;
	while ((d = readdir(dir)) != NULL) {
		if (strncmp(d->d_name, "uhid", 4) != 0) continue;
		snprintf(buf, sizeof(buf), "/dev/%s", d->d_name);
		fd = open(buf, O_RDWR);
		if (fd < 0) continue;
		r = ioctl(fd, USB_GET_DEVICEINFO, &info);
		if (r < 0) {
			// NetBSD: added in 2004
			// OpenBSD: added November 23, 2009
			// FreeBSD: missing (FreeBSD 8.0) - USE_LIBUSB works!
			die("Error: your uhid driver does not support"
			  " USB_GET_DEVICEINFO, please upgrade!\n");
			close(fd);
			closedir(dir);
			exit(1);
		}
		//printf("%s: v=%d, p=%d\n", buf, info.udi_vendorNo, info.udi_productNo);
		if (info.udi_vendorNo == vid && info.udi_productNo == pid) {
			closedir(dir);
			return fd;
		}
		close(fd);
	}
	closedir(dir);
	return -1;
}

static int uhid_teensy_fd = -1;

int teensy_open(void)
{
	teensy_close();
	uhid_teensy_fd = open_usb_device(0x16C0, 0x0478);
	if (uhid_teensy_fd < 0) return 0;
	return 1;
}

int teensy_write(void *buf, int len, double timeout)
{
	int r;

	// TODO: imeplement timeout... how??
	r = write(uhid_teensy_fd, buf, len);
	if (r == len) return 1;
	return 0;
}

void teensy_close(void)
{
	if (uhid_teensy_fd >= 0) {
		close(uhid_teensy_fd);
		uhid_teensy_fd = -1;
	}
}

int hard_reboot(void)
{
	int r, rebootor_fd;

	rebootor_fd = open_usb_device(0x16C0, 0x0477);
	if (rebootor_fd < 0) return 0;
	r = write(rebootor_fd, "reboot", 6);
	delay(0.1);
	close(rebootor_fd);
	if (r == 6) return 1;
	return 0;
}

#endif



/****************************************************************/
/*                                                              */
/*                     Read Intel Hex File                      */
/*                                                              */
/****************************************************************/

// the maximum flash image size we can support
// chips with larger memory may be used, but only this
// much intel-hex data can be loaded into memory!
#define MAX_MEMORY_SIZE 0x40000

static unsigned char firmware_image[MAX_MEMORY_SIZE];
static unsigned char firmware_mask[MAX_MEMORY_SIZE];
static int end_record_seen=0;
static int byte_count;
static unsigned int extended_addr = 0;
static int parse_hex_line(char *line);

int read_intel_hex(const char *filename)
{
	FILE *fp;
	int i, lineno=0;
	char buf[1024];

	byte_count = 0;
	end_record_seen = 0;
	for (i=0; i<MAX_MEMORY_SIZE; i++) {
		firmware_image[i] = 0xFF;
		firmware_mask[i] = 0;
	}
	extended_addr = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		//printf("Unable to read file %s\n", filename);
		return -1;
	}
	while (!feof(fp)) {
		*buf = '\0';
		if (!fgets(buf, sizeof(buf), fp)) break;
		lineno++;
		if (*buf) {
			if (parse_hex_line(buf) == 0) {
				printf("Warning, HEX parse error line %d\n", lineno);
				return -2;
			}
		}
		if (end_record_seen) break;
		if (feof(stdin)) break;
	}
	fclose(fp);
	return byte_count;
}


/* from ihex.c, at http://www.pjrc.com/tech/8051/pm2_docs/intel-hex.html */

/* parses a line of intel hex code, stores the data in bytes[] */
/* and the beginning address in addr, and returns a 1 if the */
/* line was valid, or a 0 if an error occured.  The variable */
/* num gets the number of bytes that were stored into bytes[] */


int
parse_hex_line(char *line)
{
	int addr, code, num;
	int sum, len, cksum, i;
	char *ptr;

	num = 0;
	if (line[0] != ':') return 0;
	if (strlen(line) < 11) return 0;
	ptr = line+1;
	if (!sscanf(ptr, "%02x", &len)) return 0;
	ptr += 2;
	if ((int)strlen(line) < (11 + (len * 2)) ) return 0;
	if (!sscanf(ptr, "%04x", &addr)) return 0;
	ptr += 4;
	  /* printf("Line: length=%d Addr=%d\n", len, addr); */
	if (!sscanf(ptr, "%02x", &code)) return 0;
	if (addr + extended_addr + len >= MAX_MEMORY_SIZE) return 0;
	ptr += 2;
	sum = (len & 255) + ((addr >> 8) & 255) + (addr & 255) + (code & 255);
	if (code != 0) {
		if (code == 1) {
			end_record_seen = 1;
			return 1;
		}
		if (code == 2 && len == 2) {
			if (!sscanf(ptr, "%04x", &i)) return 1;
			ptr += 4;
			sum += ((i >> 8) & 255) + (i & 255);
			if (!sscanf(ptr, "%02x", &cksum)) return 1;
			if (((sum & 255) + (cksum & 255)) & 255) return 1;
			extended_addr = i << 4;
			//printf("ext addr = %05X\n", extended_addr);
		}
		if (code == 4 && len == 2) {
			if (!sscanf(ptr, "%04x", &i)) return 1;
			ptr += 4;
			sum += ((i >> 8) & 255) + (i & 255);
			if (!sscanf(ptr, "%02x", &cksum)) return 1;
			if (((sum & 255) + (cksum & 255)) & 255) return 1;
			extended_addr = i << 16;
			//printf("ext addr = %08X\n", extended_addr);
		}
		return 1;       // non-data line
	}
	byte_count += len;
	while (num != len) {
		if (sscanf(ptr, "%02x", &i) != 1) return 0;
		i &= 255;
		firmware_image[addr + extended_addr + num] = i;
		firmware_mask[addr + extended_addr + num] = 1;
		ptr += 2;
		sum += i;
		(num)++;
		if (num >= 256) return 0;
	}
	if (!sscanf(ptr, "%02x", &cksum)) return 0;
	if (((sum & 255) + (cksum & 255)) & 255) return 0; /* checksum error */
	return 1;
}

int ihex_bytes_within_range(int begin, int end)
{
	int i;

	if (begin < 0 || begin >= MAX_MEMORY_SIZE ||
	   end < 0 || end >= MAX_MEMORY_SIZE) {
		return 0;
	}
	for (i=begin; i<=end; i++) {
		if (firmware_mask[i]) return 1;
	}
	return 0;
}

void ihex_get_data(int addr, int len, unsigned char *bytes)
{
	int i;

	if (addr < 0 || len < 0 || addr + len >= MAX_MEMORY_SIZE) {
		for (i=0; i<len; i++) {
			bytes[i] = 255;
		}
		return;
	}
	for (i=0; i<len; i++) {
		if (firmware_mask[addr]) {
			bytes[i] = firmware_image[addr];
		} else {
			bytes[i] = 255;
		}
		addr++;
	}
}

int memory_is_blank(int addr, int block_size)
{
	if (addr < 0 || addr > MAX_MEMORY_SIZE) return 1;

	while (block_size && addr < MAX_MEMORY_SIZE) {
		if (firmware_mask[addr] && firmware_image[addr] != 255) return 0;
		addr++;
		block_size--;
	}
	return 1;
}




/****************************************************************/
/*                                                              */
/*                       Misc Functions                         */
/*                                                              */
/****************************************************************/

int printf_verbose(const char *format, ...)
{
	va_list ap;
	int r;

	va_start(ap, format);
	if (verbose) {
		r = vprintf(format, ap);
		fflush(stdout);
		return r;
	}
	return 0;
}

void delay(double seconds)
{
	#ifdef WIN32
	Sleep(seconds * 1000.0);
	#else
	usleep(seconds * 1000000.0);
	#endif
}

void die(const char *str, ...)
{
	va_list  ap;

	va_start(ap, str);
	vfprintf(stderr, str, ap);
	fprintf(stderr, "\n");
	exit(1);
}

#if defined(WIN32)
#define strcasecmp stricmp
#endif

void parse_options(int argc, char **argv)
{
	int i;
	const char *arg;

	for (i=1; i<argc; i++) {
		arg = argv[i];
		//printf("arg: %s\n", arg);
		if (*arg == '-') {
			if (strcmp(arg, "-w") == 0) {
				wait_for_device_to_appear = 1;
			} else if (strcmp(arg, "-r") == 0) {
				hard_reboot_device = 1;
			} else if (strcmp(arg, "-n") == 0) {
				reboot_after_programming = 0;
			} else if (strcmp(arg, "-v") == 0) {
				verbose = 1;
			} else if (strncmp(arg, "-mmcu=", 6) == 0) {
				if (strcasecmp(arg+6, "at90usb162") == 0) {
					code_size = 15872;
					block_size = 128;
				} else if (strcasecmp(arg+6, "atmega32u4") == 0) {
					code_size = 32256;
					block_size = 128;
				} else if (strcasecmp(arg+6, "at90usb646") == 0) {
					code_size = 64512;
					block_size = 256;
				} else if (strcasecmp(arg+6, "at90usb1286") == 0) {
					code_size = 130048;
					block_size = 256;
#if defined(USE_LIBUSB)
				} else if (strcasecmp(arg+6, "mk20dx128") == 0) {
					code_size = 131072;
					block_size = 1024;
				} else if (strcasecmp(arg+6, "mk20dx256") == 0) {
					code_size = 262144;
					block_size = 1024;
#endif
				} else {
					die("Unknown MCU type\n");
				}
			}
		} else {
			filename = argv[i];
		}
	}
}

