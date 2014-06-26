/* -*- mode: ld-script; -*- */
/*
 * Linker script for MK20DX128VLF5
 */
MEMORY
{
	FLASH (rx) : ORIGIN = 3K, LENGTH = 128K-3K
	RAM  (rwx) : ORIGIN = 0x20000000 - 16K / 2, LENGTH = 16K
}

/* Section Definitions */
SECTIONS
{
	.text :
	{
		. = 0;
		KEEP(* (.vectors))
		*(.startup*)
		*(.text*)
		*(.rodata*)
		. = ALIGN(4);
		KEEP(*(.init))
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP (*(.preinit_array))
		__preinit_array_end = .;
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
	} > FLASH = 0xFF

	.ARM.exidx :
	{
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} > FLASH
	_etext = .;

	.usbdescriptortable (NOLOAD) : {
		. = ALIGN(512);
		*(.usbdescriptortable*)
	} > RAM

	.dmabuffers (NOLOAD) : {
		. = ALIGN(4);
		*(.dmabuffers*)
	} > RAM

	.usbbuffers (NOLOAD) : {
		. = ALIGN(4);
		*(.usbbuffers*)
	} > RAM

	.data : AT (_etext) {
		. = ALIGN(4);
		_sdata = .;
		*(.data*)
		. = ALIGN(4);
		_edata = .;
	} > RAM

	.noinit (NOLOAD) : {
		*(.noinit*)
	} > RAM

	.bss : {
		. = ALIGN(4);
		_sbss = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
		__bss_end = .;
	} > RAM

	_estack = ORIGIN(RAM) + LENGTH(RAM);
}

