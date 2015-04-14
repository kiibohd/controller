/* Copyright (C) 2015 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <print.h>

// Local Includes
#include "lcd_scan.h"



// ----- Defines -----

#define LCD_TOTAL_VISIBLE_PAGES 4
#define LCD_PAGE_LEN 132



// ----- Macros -----

// Number of entries in the SPI0 TxFIFO
#define SPI0_TxFIFO_CNT ( ( SPI0_SR & SPI_SR_TXCTR ) >> 12 )



// ----- Structs -----

// ----- Function Declarations -----

// CLI Functions
void cliFunc_lcdCmd( char* args );
void cliFunc_lcdInit( char* args );
void cliFunc_lcdTest( char* args );



// ----- Variables -----

// Full Toggle State
uint8_t cliFullToggleState = 0;

// Normal/Reverse Toggle State
uint8_t cliNormalReverseToggleState = 0;

// Scan Module command dictionary
CLIDict_Entry( lcdCmd,      "Send byte via SPI, second argument enables a0. Defaults to control." );
CLIDict_Entry( lcdInit,     "Re-initialize the LCD display." );
CLIDict_Entry( lcdTest,     "Test out the LCD display." );

CLIDict_Def( lcdCLIDict, "ST LCD Module Commands" ) = {
	CLIDict_Item( lcdCmd ),
	CLIDict_Item( lcdInit ),
	CLIDict_Item( lcdTest ),
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Interrupt Functions -----



// ----- Functions -----

inline void SPI_setup()
{
	// Enable SPI internal clock
	SIM_SCGC6 |= SIM_SCGC6_SPI0;

	// Setup MOSI (SOUT) and SCLK (SCK)
	PORTC_PCR6 = PORT_PCR_DSE | PORT_PCR_MUX(2);
	PORTC_PCR5 = PORT_PCR_DSE | PORT_PCR_MUX(2);

	// Setup SS (PCS)
	PORTC_PCR4 = PORT_PCR_DSE | PORT_PCR_MUX(2);

	// Master Mode, CS0
	SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(1);

	// DSPI Clock and Transfer Attributes
	// Frame Size: 8 bits
	// MSB First
	// CLK Low by default
	SPI0_CTAR0 = SPI_CTAR_FMSZ(7)
		| SPI_CTAR_ASC(7)
		| SPI_CTAR_DT(7)
		| SPI_CTAR_CSSCK(7)
		| SPI_CTAR_PBR(0) | SPI_CTAR_BR(7);
}

// Write buffer to SPI FIFO
void SPI_write( uint8_t *buffer, uint8_t len )
{

	for ( uint8_t byte = 0; byte < len; byte++ )
	{
		// Wait for SPI TxFIFO to have 4 or fewer entries
		while ( !( SPI0_SR & SPI_SR_TFFF ) )
			delayMicroseconds(10);

		// Write byte to TxFIFO
		// CS0, CTAR0
		SPI0_PUSHR = ( buffer[ byte ] & 0xff ) | SPI_PUSHR_PCS(1);

		// Indicate transfer has completed
		while ( !( SPI0_SR & SPI_SR_TCF ) );
		SPI0_SR |= SPI_SR_TCF;
	}
}

// Write to a control register
void LCD_writeControlReg( uint8_t byte )
{
	// Wait for TxFIFO to be empt
	while ( SPI0_TxFIFO_CNT != 0 );

	// Set A0 low to enter control register mode
	GPIOC_PCOR |= (1<<7);

	// Write byte to SPI FIFO
	SPI_write( &byte, 1 );

	// Wait for TxFIFO to be empty
	while ( SPI0_TxFIFO_CNT != 0 );

	// Make sure data has transferred
	delayMicroseconds(10); // XXX Adjust if SPI speed changes

	// Set A0 high to go back to display register mode
	GPIOC_PSOR |= (1<<7);
}

// Write to display register
// Pages 0-7 normal display
// Page  8   icon buffer
void LCD_writeDisplayReg( uint8_t page, uint8_t *buffer, uint8_t len )
{
	// Set the register page
	LCD_writeControlReg( 0xB0 | ( 0x0F & page ) );

	// Write buffer to SPI
	SPI_write( buffer, len );
}

inline void LCD_clearPage( uint8_t page )
{
	// Set the register page
	LCD_writeControlReg( 0xB0 | ( 0x0F & page ) );

	// Set display start line
	LCD_writeControlReg( 0x40 );

	// Reset Column Address
	LCD_writeControlReg( 0x10 );
	LCD_writeControlReg( 0x00 );

	for ( uint8_t page_reg = 0; page_reg < LCD_PAGE_LEN; page_reg++ )
	{
		uint8_t byte = 0;

		// Write buffer to SPI
		SPI_write( &byte, 1 );
	}

	// Wait for TxFIFO to be empty
	while ( SPI0_TxFIFO_CNT != 0 );
}

// Clear Display
void LCD_clear()
{
	// Setup each page
	for ( uint8_t page = 0; page < LCD_TOTAL_VISIBLE_PAGES; page++ )
	{
		LCD_clearPage( page );
	}

	// Reset Page, Start Line, and Column Address
	// Page
	LCD_writeControlReg( 0xB0 );

	// Start Line
	LCD_writeControlReg( 0x40 );

	// Reset Column Address
	LCD_writeControlReg( 0x10 );
	LCD_writeControlReg( 0x00 );
}

// Intialize display
void LCD_initialize()
{
	// ADC Select (Normal)
	LCD_writeControlReg( 0xA0 );

	// LCD Off
	LCD_writeControlReg( 0xAE );

	// COM Scan Output Direction
	LCD_writeControlReg( 0xC0 );

	// LCD Bias (1/6 bias)
	LCD_writeControlReg( 0xA2 );

	// Power Supply Operating Mode (Internal Only)
	LCD_writeControlReg( 0x2F );

	// Internal Rb/Ra Ratio
	LCD_writeControlReg( 0x26 );

	// Reset
	LCD_writeControlReg( 0xE2 );

	// Electric volume mode set, and value
	LCD_writeControlReg( 0x81 );
	LCD_writeControlReg( 0x00 );

	// LCD On
	LCD_writeControlReg( 0xAF );

	// Clear Display RAM
	LCD_clear();
}

// Setup
inline void LCD_setup()
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( lcdCLIDict, lcdCLIDictName );

	// Initialize SPI
	SPI_setup();


	// Setup Register Control Signal (A0)
	// Start in display register mode (1)
	GPIOC_PDDR |= (1<<7);
	PORTC_PCR7 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PSOR |= (1<<7);

	// Setup LCD Reset pin (RST)
	// 0 - Reset, 1 - Normal Operation
	// Start in normal mode (1)
	GPIOC_PDDR |= (1<<8);
	PORTC_PCR8 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	GPIOC_PSOR |= (1<<8);

	// Run LCD intialization sequence
	LCD_initialize();
}


// LCD State processing loop
inline uint8_t LCD_scan()
{
	// NOP - Screen Refresh
	//LCD_writeControlReg( 0xE3 );
	return 0;
}



// ----- CLI Command Functions -----

void cliFunc_lcdInit( char* args )
{
	print( NL ); // No \r\n by default after the command is entered
	LCD_initialize();
}

void cliFunc_lcdTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	//LCD_initialize();
	// Test pattern
	uint8_t pattern[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	//uint8_t pattern[] = { 0xFF, 0x00, 0x96, 0xFF, 0x00, 0xFF, 0x00 };

	// Write to page D0
	LCD_writeDisplayReg( 0, pattern, sizeof( pattern ) );
}

void cliFunc_lcdCmd( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	print( NL ); // No \r\n by default after the command is entered

	curArgs = arg2Ptr; // Use the previous 2nd arg pointer to separate the next arg from the list
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// No args
	if ( *arg1Ptr == '\0' )
		return;

	// SPI Command
	uint8_t cmd = (uint8_t)numToInt( arg1Ptr );

	curArgs = arg2Ptr; // Use the previous 2nd arg pointer to separate the next arg from the list
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Single Arg
	if ( *arg1Ptr == '\0' )
		goto cmd;

	// TODO Deal with a0
cmd:
	info_msg("Sending - ");
	printHex( cmd );
	print( NL );
	LCD_writeControlReg( cmd );
}

