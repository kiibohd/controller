/* Copyright (C) 2015-2016 by Jacob Alexander
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
#include <kll_defs.h>
#include <led.h>
#include <print.h>

// Interconnect module if compiled in
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// Local Includes
#include "lcd_scan.h"

// ----- Defines -----

#define LCD_TOTAL_VISIBLE_PAGES 4
#define LCD_TOTAL_PAGES 9
#define LCD_PAGE_LEN 128
#define LCD_WIDTH 128
#define LCD_HEIGHT 32


// ----- Macros -----

// Number of entries in the SPI0 TxFIFO
#define SPI0_TxFIFO_CNT ( ( SPI0_SR & SPI_SR_TXCTR ) >> 12 )



// ----- Structs -----

// ----- Function Declarations -----

// CLI Functions
void cliFunc_lcdCmd      ( char* args );
void cliFunc_lcdColor    ( char* args );
void cliFunc_lcdDisp     ( char* args );
void cliFunc_lcdInit     ( char* args );
void cliFunc_lcdTest     ( char* args );
void cliFunc_lcdTextOut  ( char* args );
void cliFunc_ttyPrint    ( char* args );
void cliFunc_ttyScrollUp ( char* args );




// ----- Variables -----

// Default Image - Displays on startup
const uint8_t STLcdDefaultImage[] = { STLcdDefaultImage_define };

const uint8_t STLcdDefaultFont[] = { STLcdDefaultFont_define };
const uint8_t STLcdDefaultFontWidth = STLcdDefaultFontWidth_define;
const uint8_t STLcdDefaultFontHeight = STLcdDefaultFontHeight_define;
const uint8_t STLcdDefaultFontSize = STLcdDefaultFontSize_define;
const uint8_t STLcdDefaultFontLength = STLcdDefaultFontLength_define;



// Full Toggle State
uint8_t cliFullToggleState = 0;

// Normal/Reverse Toggle State
uint8_t cliNormalReverseToggleState = 0;

// Scan Module command dictionary
CLIDict_Entry( lcdCmd,      "Send byte via SPI, second argument enables a0. Defaults to control." );
CLIDict_Entry( lcdColor,    "Set backlight color. 3 16-bit numbers: R G B. i.e. 0xFFF 0x1444 0x32" );
CLIDict_Entry( lcdDisp,     "Write byte(s) to given page starting at given address. i.e. 0x1 0x5 0xFF 0x00" );
CLIDict_Entry( lcdInit,     "Re-initialize the LCD display." );
CLIDict_Entry( lcdTest,     "Test out the LCD display." );
CLIDict_Entry( lcdTextOut,  "Output text to the LCD at the given x-y coordinate.");
CLIDict_Entry( ttyPrint,    "Output text to the LCD." );
CLIDict_Entry( ttyScrollUp, "Scroll up given lines on the LCD." );

CLIDict_Def( lcdCLIDict, "ST LCD Module Commands" ) = {
	CLIDict_Item( lcdCmd ),
	CLIDict_Item( lcdColor ),
	CLIDict_Item( lcdDisp ),
	CLIDict_Item( lcdInit ),
	CLIDict_Item( lcdTest ),
	CLIDict_Item( lcdTextOut),
	CLIDict_Item( ttyPrint ),
	CLIDict_Item( ttyScrollUp ),
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
// Page	 8   icon buffer
void LCD_writeDisplayReg( uint8_t page, uint8_t *buffer, uint8_t len )
{
	// Set the register page
	LCD_writeControlReg( 0xB0 | ( 0x0F & page ) );

	// Set display start line
	LCD_writeControlReg( 0x40 );

	// Reset Column Address
	LCD_writeControlReg( 0x10 );
	LCD_writeControlReg( 0x00 );

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
	for ( uint8_t page = 0; page < LCD_TOTAL_PAGES; page++ )
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

	// Write default image to LCD
	for ( uint8_t page = 0; page < LCD_TOTAL_VISIBLE_PAGES; page++ )
		LCD_writeDisplayReg( page, (uint8_t*)&STLcdDefaultImage[page * LCD_PAGE_LEN], LCD_PAGE_LEN );

	// Setup Backlight
	SIM_SCGC6 |= SIM_SCGC6_FTM0;
	FTM0_CNT = 0; // Reset counter

	// PWM Period
	// 16-bit maximum
	FTM0_MOD = 0xFFFF;

	// Set FTM to PWM output - Edge Aligned, Low-true pulses
	FTM0_C0SC = 0x24; // MSnB:MSnA = 10, ELSnB:ELSnA = 01
	FTM0_C1SC = 0x24;
	FTM0_C2SC = 0x24;

	// Base FTM clock selection (72 MHz system clock)
	// @ 0xFFFF period, 72 MHz / (0xFFFF * 2) = Actual period
	// Higher pre-scalar will use the most power (also look the best)
	// Pre-scalar calculations
	// 0 -	    72 MHz -> 549 Hz
	// 1 -	    36 MHz -> 275 Hz
	// 2 -	    18 MHz -> 137 Hz
	// 3 -	     9 MHz ->  69 Hz (Slightly visible flicker)
	// 4 -	 4 500 kHz ->  34 Hz (Visible flickering)
	// 5 -	 2 250 kHz ->  17 Hz
	// 6 -	 1 125 kHz ->	9 Hz
	// 7 - 562 500	Hz ->	4 Hz
	// Using a higher pre-scalar without flicker is possible but FTM0_MOD will need to be reduced
	// Which will reduce the brightness range

	// System clock, /w prescalar setting
	FTM0_SC = FTM_SC_CLKS(1) | FTM_SC_PS( STLcdBacklightPrescalar_define );

	// Red
	FTM0_C0V = STLcdBacklightRed_define;
	PORTC_PCR1 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(4);

	// Green
	FTM0_C1V = STLcdBacklightGreen_define;
	PORTC_PCR2 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(4);

	// Blue
	FTM0_C2V = STLcdBacklightBlue_define;
	PORTC_PCR3 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(4);
}

static uint32_t STLcdSyncStartMillis;
static uint32_t STLcdSyncStartTicks;

// Display buffer
uint8_t STLcdBuffer [ LCD_PAGE_LEN * LCD_TOTAL_VISIBLE_PAGES ];
uint8_t STLcdSyncBuffer [ LCD_PAGE_LEN * LCD_TOTAL_VISIBLE_PAGES ];


// Bounding box.
static uint8_t STLcdUpdateXMin = LCD_WIDTH, STLcdUpdateXMax = 0, STLcdUpdateYMin = LCD_HEIGHT, STLcdUpdateYMax = 0;
static uint8_t STLcdSync = 0;
static uint8_t STLcdSyncPageMin, STLcdSyncPageMax, STLcdSyncColumnMin, STLcdSyncColumnMax;

static void STLcd_updateBoundingBox( uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax ) {
	if ( xmin < STLcdUpdateXMin ) STLcdUpdateXMin = xmin;
	if ( xmax > STLcdUpdateXMax ) STLcdUpdateXMax = xmax;
	if ( ymin < STLcdUpdateYMin ) STLcdUpdateYMin = ymin;
	if ( ymax > STLcdUpdateYMax ) STLcdUpdateYMax = ymax;
}

void STLcd_clear( void ) {
	memset( STLcdBuffer, 0, LCD_PAGE_LEN * LCD_TOTAL_VISIBLE_PAGES );
	STLcd_updateBoundingBox( 0, 0, LCD_WIDTH, LCD_HEIGHT );
}

static uint8_t STLcdSyncPageCurrent;
static uint8_t *STLcdSyncBufferColumnCurrent;
static uint8_t *STLcdSyncBufferColumnMin, *STLcdSyncBufferColumnMax;
static uint32_t STLcdSyncStage;

#define STAGE_WRITE_START 15

void STLcd_blockingSync(void) {
	for ( ; STLcdSyncPageCurrent < STLcdSyncPageMax;
		  STLcdSyncPageCurrent++, STLcdSyncBufferColumnMin += LCD_PAGE_LEN )
	{
		LCD_writeControlReg( 0xB0 | STLcdSyncPageCurrent );
		LCD_writeControlReg( 0x10 | STLcdSyncColumnMin >> 4 );
		LCD_writeControlReg( 0x10 | ( STLcdSyncColumnMin & 0x0f ) );
		SPI_write( STLcdSyncBufferColumnMin, STLcdSyncColumnMax - STLcdSyncColumnMin );
	}
}

// a non-blocking sync function
void STLcd_sync() {
	for ( ;; )
	{
		switch(STLcdSyncStage)
		{
		// Verify SPI0 TxFIFO is not full, then enable LCD configuration registers (A0 to Low)
		case 0:
			if ( SPI0_TxFIFO_CNT != 0 ) // while ( SPI0_TxFIFO_CNT != 0 );
				return;
			GPIOC_PCOR |= (1<<7);
			STLcdSyncStage++;

		// Verify SPI0 TxFIFO has 4 or fewer entries, then write page number to TxFIFO (CS0, CTAR0)
		case 1: // Begin SPI_write( &byte, 1);
			if ( !( SPI0_SR & SPI_SR_TFFF ) )
				return;
			SPI0_PUSHR = ( 0xB0 | STLcdSyncPageCurrent ) | SPI_PUSHR_PCS(1);
			STLcdSyncStage++;

		// TODO: Verify the transfer has completed?
		case 2:
			if ( !( SPI0_SR & SPI_SR_TCF ) )
				return;
			SPI0_SR |= SPI_SR_TCF;
			STLcdSyncStage++; // End SPI_write

		// Verify SPIO TxFIFO is empty, then save the current time.
		case 3:
			if ( SPI0_TxFIFO_CNT != 0 ) // while ( SPI0_TxFIFO_CNT != 0 );
				return;
			STLcdSyncStage++;
			STLcdSyncStartTicks = ticks();
			STLcdSyncStartMillis = millis();

		// Verify that 10 microseconds have passed, then enable LCD display registors (A0 to high)
		case 4:
			// delayMicroseconds(10);
			if ( !isTicksPassed( STLcdSyncStartMillis, STLcdSyncStartTicks, F_CPU / 1000000 * 10 ) )
				return;
			GPIOC_PSOR |= (1<<7);
			STLcdSyncStage++; // End LCD_writeControlReg( 0xB0 | page);

		// TODO: maybe these lines is not need? Because the last command also writes to the control register.
		case 5: // Begin LCD_writeControlReg( 0x10 | STLcdSyncColumnMin >> 4);
			if ( SPI0_TxFIFO_CNT != 0 ) // while ( SPI0_TxFIFO_CNT != 0 );
				return;
			GPIOC_PCOR |= (1<<7);
			STLcdSyncStage++;

		//  Verify SPI0 TxFIFO has 4 or fewer entries,
		//  then write the high half of column number to TxFIFO (CS0, CTAR0)
		case 6: // Begin SPI_write( &byte, 1);
			if ( !( SPI0_SR & SPI_SR_TFFF) )
				return;
			SPI0_PUSHR = ( 0x10 | STLcdSyncColumnMin >> 4 ) | SPI_PUSHR_PCS(1);
			STLcdSyncStage++;

		// TODO: Verify the transfer has completed?
		case 7:
			if ( !( SPI0_SR & SPI_SR_TCF ) )
				return;
			SPI0_SR |= SPI_SR_TCF;
			STLcdSyncStage++; // End SPI_write

		// Verify SPIO TxFIFO is empty, then save the current time.
		case 8:
			if ( SPI0_TxFIFO_CNT != 0 ) // while ( SPI0_TxFIFO_CNT != 0 );
				return;
			STLcdSyncStage++;
			STLcdSyncStartTicks = ticks();
			STLcdSyncStartMillis = millis();

		// Verify that 10 microseconds have passed, then enable LCD display registors (A0 to high)
		case 9:
			// delayMicroseconds(10);
			if ( !isTicksPassed( STLcdSyncStartMillis, STLcdSyncStartTicks, F_CPU / 1000000 * 10 ) )
				return;
			GPIOC_PSOR |= (1<<7);
			STLcdSyncStage++; // End LCD_writeControlReg( 0x10 | STLcdSyncColumnCurrent >> 4);

		// TODO: maybe these lines is not need? Because the last command also writes to the control register.
		case 10: // Begin LCD_writeControlReg( 0x00 | STLcdSyncColumnCurrent & 0x0f);
			if ( SPI0_TxFIFO_CNT != 0 ) // while ( SPI0_TxFIFO_CNT != 0 );
				return;
			GPIOC_PCOR |= (1<<7);
			STLcdSyncStage++;

		//  Verify SPI0 TxFIFO has 4 or fewer entries,
		//  then write the lower half of column number to TxFIFO (CS0, CTAR0)
		case 11: // Begin SPI_write( &byte, 1);
			if ( !( SPI0_SR & SPI_SR_TFFF) )
				return;
			SPI0_PUSHR = ( 0x00 | ( STLcdSyncColumnMin & 0x0f ) ) | SPI_PUSHR_PCS(1);
			STLcdSyncStage++;

		// TODO: Verify the transfer has completed?
		case 12:
			if ( !( SPI0_SR & SPI_SR_TCF ) )
				return;
			SPI0_SR |= SPI_SR_TCF;
			STLcdSyncStage++; // End SPI_write

		// Verify SPIO TxFIFO is empty, then save the current time.
		case 13:
			if ( SPI0_TxFIFO_CNT != 0 ) // while ( SPI0_TxFIFO_CNT != 0 );
				return;
			STLcdSyncStage++;
			STLcdSyncStartTicks = ticks();
			STLcdSyncStartMillis = millis();

		// Verify that 10 microseconds have passed, then enable LCD display registors (A0 to high),
		// then begin to write the buffer to SPI.
		case 14:
			// delayMicroseconds(10);
			if ( !isTicksPassed( STLcdSyncStartMillis, STLcdSyncStartTicks, F_CPU / 1000000 * 10 ) )
				return;
			GPIOC_PSOR |= (1<<7);
			STLcdSyncStage++; // End LCD_writeControlReg( 0x00 | (STLcdSyncColumnCurrent & 0x0f));
			STLcdSyncBufferColumnCurrent = STLcdSyncBufferColumnMin;
			// Begin SPI_write( STLcdSyncBufferColumnMin,
			//                  STLcdSyncColumnMax - STLcdSyncColumnMin);

		// If the current page has been outputed,
		// then change to next page or finish the sync( if it's the last page).
		case STAGE_WRITE_START:
			if ( STLcdSyncBufferColumnCurrent >= STLcdSyncBufferColumnMax )
			{ // next page
				STLcdSyncStage = 0;
				STLcdSyncPageCurrent++;
				if ( STLcdSyncPageCurrent >= STLcdSyncPageMax )
				{ // has finished
					STLcdSync = 0;
					return;
				}
				else{
					STLcdSyncBufferColumnMin += LCD_PAGE_LEN;
					STLcdSyncBufferColumnMax += LCD_PAGE_LEN;
					break;
				}
			}
			STLcdSyncStage++;

		//  Verify SPI0 TxFIFO has 4 or fewer entries,
		//  then write the byte of current column to TxFIFO (CS0, CTAR0)
		case 16:
			if( !( SPI0_SR & SPI_SR_TFFF) )
				return;
			SPI0_PUSHR = ( *STLcdSyncBufferColumnCurrent ) | SPI_PUSHR_PCS(1);
			STLcdSyncStage++;

		// TODO: Verify the transfer has completed?
		// then changed to the next column of current page and go back to the start of this loop.
		case 17:
			if( !( SPI0_SR & SPI_SR_TCF ) )
				return;
			SPI0_SR |= SPI_SR_TCF;
			STLcdSyncBufferColumnCurrent++;
			STLcdSyncStage=STAGE_WRITE_START; // Loop back to STAGE_WRITE_START
			break;
		}
	}
}


// LCD State processing loop
inline uint8_t LCD_scan()
{
	if ( STLcdSync )
	{
		STLcd_sync();
	}
	else
	{
		if ( STLcdUpdateYMin >= STLcdUpdateYMax )
			return 0;
		STLcdSyncPageMin = STLcdUpdateYMin >> 3;
		STLcdSyncPageMax = ( STLcdUpdateYMax + 7 ) >> 3;
		STLcdSyncPageCurrent = STLcdSyncPageMin;

		STLcdSyncColumnMin = STLcdUpdateXMin;
		STLcdSyncColumnMax = STLcdUpdateXMax;

		STLcdSyncBufferColumnMin = STLcdSyncBuffer + STLcdSyncPageCurrent * LCD_PAGE_LEN + STLcdSyncColumnMin;
		STLcdSyncBufferColumnMax = STLcdSyncBuffer + STLcdSyncPageCurrent * LCD_PAGE_LEN + STLcdSyncColumnMax;

		memcpy( STLcdSyncBuffer + STLcdSyncPageMin * LCD_PAGE_LEN, STLcdBuffer + STLcdSyncPageMin * LCD_PAGE_LEN,
			( STLcdSyncPageMax - STLcdSyncPageMin ) * LCD_PAGE_LEN );

		STLcdSync = 1;
		STLcdSyncStage = 0;
		//STLcd_syncBlocking();
		STLcd_sync();

		STLcdUpdateXMin = LCD_WIDTH - 1;
		STLcdUpdateXMax = 0;
		STLcdUpdateYMin = LCD_HEIGHT - 1;
		STLcdUpdateYMax = 0;
	}
	return 0;
}


// Signal from parent Scan Module that available current has changed
// current - mA
void LCD_currentChange( unsigned int current )
{
	// TODO - Power savings?
}



// ----- Capabilities -----

// Takes 1 8 bit length and 4 16 bit arguments, each corresponding to a layer index
// Ordered from top to bottom
// The first argument indicates how many numbers to display (max 4), set to 0 to load default image
uint16_t LCD_layerStackExact[4];
uint8_t LCD_layerStackExact_size = 0;
typedef struct LCD_layerStackExact_args {
	uint8_t numArgs;
	uint16_t layers[4];
} LCD_layerStackExact_args;
void LCD_layerStackExact_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("LCD_layerStackExact_capability(num,layer1,layer2,layer3,layer4)");
		return;
	}

	// Read arguments
	LCD_layerStackExact_args *stack_args = (LCD_layerStackExact_args*)args;

	// Number data for LCD
	const uint8_t numbers[10][128] = {
		{ STLcdNumber0_define },
		{ STLcdNumber1_define },
		{ STLcdNumber2_define },
		{ STLcdNumber3_define },
		{ STLcdNumber4_define },
		{ STLcdNumber5_define },
		{ STLcdNumber6_define },
		{ STLcdNumber7_define },
		{ STLcdNumber8_define },
		{ STLcdNumber9_define },
	};

	// Color data for numbers
	const uint16_t colors[10][3] = {
		{ STLcdNumber0Color_define },
		{ STLcdNumber1Color_define },
		{ STLcdNumber2Color_define },
		{ STLcdNumber3Color_define },
		{ STLcdNumber4Color_define },
		{ STLcdNumber5Color_define },
		{ STLcdNumber6Color_define },
		{ STLcdNumber7Color_define },
		{ STLcdNumber8Color_define },
		{ STLcdNumber9Color_define },
	};

	// Only display if there are layers active
	if ( stack_args->numArgs > 0 )
	{
		// Set the color according to the "top-of-stack" layer
		uint16_t layerIndex = stack_args->layers[0];
		FTM0_C0V = colors[ layerIndex ][0];
		FTM0_C1V = colors[ layerIndex ][1];
		FTM0_C2V = colors[ layerIndex ][2];

		// Iterate through each of the pages
		// XXX Many of the values here are hard-coded
		//	   Eventually a proper font rendering engine should take care of things like this... -HaaTa
		STLcd_clear();
		for ( uint8_t page = 0; page < LCD_TOTAL_VISIBLE_PAGES; page++ )
		{
			uint8_t offset = 0;
			// Write data
			for ( uint16_t layer = 0; layer < stack_args->numArgs; layer++ )
			{
				layerIndex = stack_args->layers[ layer ];

				// Default to 0, if over 9
				if ( layerIndex > 9 )
				{
					layerIndex = 0;
				}
				memcpy(STLcdBuffer + page * LCD_PAGE_LEN + offset,
				       &numbers[layerIndex][page * 32],
				       32);
				offset += 32;
			}
		}
	}
	else
	{
		// Set default backlight
		FTM0_C0V = STLcdBacklightRed_define;
		FTM0_C1V = STLcdBacklightGreen_define;
		FTM0_C2V = STLcdBacklightBlue_define;

		// Write default image
		memcpy(STLcdBuffer, STLcdDefaultImage, LCD_PAGE_LEN * LCD_TOTAL_VISIBLE_PAGES);
		STLcd_updateBoundingBox(0, 0, LCD_WIDTH, LCD_HEIGHT);
	}
}

// Determines the current layer stack, and sets the LCD output accordingly
// Will only work on a master node when using the interconnect (use LCD_layerStackExact_capability instead)
uint16_t LCD_layerStack_prevSize = 0;
uint16_t LCD_layerStack_prevTop	 = 0;
void LCD_layerStack_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("LCD_layerStack_capability()");
		return;
	}

	// Parse the layer stack, top to bottom
	extern uint16_t macroLayerIndexStack[];
	extern uint16_t macroLayerIndexStackSize;

	// Ignore if the stack size hasn't changed and the top of the stack is the same
	if ( macroLayerIndexStackSize == LCD_layerStack_prevSize
		 && (macroLayerIndexStackSize == 0 ||
		 macroLayerIndexStack[macroLayerIndexStackSize - 1] == LCD_layerStack_prevTop ))
	{
		return;
	}
	LCD_layerStack_prevSize = macroLayerIndexStackSize;
	LCD_layerStack_prevTop	= macroLayerIndexStack[macroLayerIndexStackSize - 1];

	LCD_layerStackExact_args stack_args;
	memset( stack_args.layers, 0, sizeof( stack_args.layers ) );

	// Use the LCD_layerStackExact_capability to set the LCD using the determined stack
	// Construct argument set for capability
	stack_args.numArgs = macroLayerIndexStackSize;
	for ( uint16_t layer = 1; layer <= macroLayerIndexStackSize; layer++ )
	{
		stack_args.layers[ layer - 1 ] = macroLayerIndexStack[ macroLayerIndexStackSize - layer ];
	}

	// Only deal with the interconnect if it has been compiled in
#if defined(ConnectEnabled_define)
	if ( Connect_master )
	{
		// generatedKeymap.h
		extern const Capability CapabilitiesList[];

		// Broadcast layerStackExact remote capability (0xFF is the broadcast id)
		Connect_send_RemoteCapability(
			0xFF,
			LCD_layerStackExact_capability_index,
			state,
			stateType,
			CapabilitiesList[ LCD_layerStackExact_capability_index ].argCount,
			(uint8_t*)&stack_args
		);
	}
#endif
	// Call LCD_layerStackExact directly
	LCD_layerStackExact_capability( state, stateType, (uint8_t*)&stack_args );
}

static uint8_t STLcdDrawMasks[8][8] =
{
	{ 0x00, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80 },
	{ 0x01, 0xfd, 0xf9, 0xf1, 0xe1, 0xc1, 0x81, 0x01 },
	{ 0x03, 0xfb, 0xf3, 0xe3, 0xc3, 0x83, 0x03, 0x03 },
	{ 0x07, 0xf7, 0xe7, 0xc7, 0x87, 0x07, 0x07, 0x07 },
	{ 0x0f, 0xef, 0xcf, 0x8f, 0x0f, 0x0f, 0x0f, 0x0f },
	{ 0x1f, 0xdf, 0x9f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f },
	{ 0x3f, 0xbf, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f },
	{ 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f }
};

void STLcd_drawBitmap( const uint8_t *bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height )
{
	uint8_t *buffer = STLcdBuffer + (y>>3) * LCD_PAGE_LEN + x;
	uint8_t maxcolumn = ( x + width <= LCD_WIDTH ) ? width : ( LCD_WIDTH - x );
	uint8_t remainheight =
		( y + height <= LCD_HEIGHT )
		? height : ( LCD_HEIGHT - y );
	uint8_t srcpages = ( remainheight + 7 ) / 8;
	uint8_t page = 0;
	if ( y & 0x07 )
	{
		uint8_t highbits = y & 0x07;

		uint8_t mask = STLcdDrawMasks[ highbits ][ ( remainheight > 7 ) ? 0 : remainheight ];
		for ( uint8_t column = 0; column < maxcolumn; column++ )
		{
			buffer[ column ] = ( buffer[ column ] & mask ) | ( bitmap[ column ] << highbits );
		}
		remainheight -= 8 - highbits;
		page++;
		for ( ; remainheight > 7; page++, remainheight -= 8 )
		{
			for ( uint8_t column = 0; column < maxcolumn; column++ )
			{
				buffer[ page * LCD_PAGE_LEN + column ] =
					( bitmap[ ( page - 1 ) * width + column ] >> ( 8 - highbits ) )
					| ( bitmap[ page * width + column ] << highbits );
			}
		}
		if ( remainheight > 0 )
		{
			mask = ~STLcdDrawMasks[ remainheight ][ 0 ];
			if ( page == srcpages )
			{
				for ( uint8_t column = 0; column < maxcolumn; column++ )
				{
					buffer[ page * LCD_PAGE_LEN + column ] =
						( bitmap[ ( page - 1 ) * width + column ] >> ( 8 - highbits ) )
						| ( buffer[ page * LCD_PAGE_LEN + column ] & mask );
				}
			}
			else
			{
				for ( uint8_t column = 0; column < maxcolumn; column++ )
				{
					buffer[ page * LCD_PAGE_LEN + column ] =
						( bitmap[ page * width + column ] << highbits )
						| ( bitmap[ ( page - 1 ) * width + column ] >> ( 8 - highbits ) )
						| ( buffer[ page * LCD_PAGE_LEN + column ] & mask );
				}
			}
		}
	}
	else
	{
		uint8_t page = 0;
		for ( page = 0; remainheight > 7; page++, remainheight -= 8 )
		{
			memcpy( buffer + page * LCD_PAGE_LEN,
				bitmap + page * width,
				maxcolumn );
		}
		if ( remainheight > 0 )
		{
			uint8_t mask = ~STLcdDrawMasks[ remainheight ][ 0 ];
			for ( uint8_t column = 0; column < maxcolumn; column++ )
			{
				uint16_t destindex = page * LCD_PAGE_LEN + column;
				buffer[ destindex ] = ( buffer[ destindex ] & mask )
					| ( bitmap[ page * width + column ] );
			}
		}
	}
}

static uint8_t TTYInitialized = 0;
static const uint8_t * TTYFont = 0;
static uint8_t TTYFontHeight = 0;
static uint8_t TTYFontWidth = 0;
static uint8_t TTYFontSize = 0;
static uint8_t TTYSpacing = 0;
static uint8_t TTYLineHeight = 0;
static uint8_t TTYFontLength = 0;
static uint8_t TTYLines = 0;
static uint8_t TTYColumns = 0;
static uint8_t TTYCurrentLine = 0;
static uint8_t TTYCurrentColumn = 0;

void TTY_initialize( const uint8_t *font, uint8_t fontSize, uint8_t fontLength,
		     uint8_t fontWidth, uint8_t fontHeight,
		     uint8_t spacing, uint8_t lineHeight )
{
	STLcd_clear();
	TTYInitialized = 1;
	TTYFont = font;
	TTYFontSize = fontSize;
	TTYFontWidth = fontWidth;
	TTYFontHeight = fontHeight;
	TTYFontLength = fontLength;
	TTYSpacing = spacing;
	TTYLineHeight = lineHeight;
	TTYLines = ( LCD_HEIGHT - fontHeight) / lineHeight + 1;
	TTYColumns = ( LCD_WIDTH - fontWidth ) / ( fontWidth + spacing ) + 1;
	TTYCurrentLine = TTYLines - 1;
	TTYCurrentColumn = 0;
}

void TTY_exit()
{
	STLcd_clear();
	TTYInitialized = 0;
}

void TTY_drawGlyph( uint8_t index )
{
	if ( index < TTYFontLength )
		return;
	STLcd_drawBitmap( TTYFont + index * TTYFontSize,
			  TTYCurrentColumn * ( TTYFontWidth + TTYSpacing ),
			  TTYCurrentLine * TTYLineHeight,
			  TTYFontWidth,
			  TTYFontHeight );
	STLcd_updateBoundingBox( TTYCurrentColumn * ( TTYFontWidth + TTYSpacing ),
				 TTYCurrentLine * TTYLineHeight,
				 TTYCurrentColumn * ( TTYFontWidth + TTYSpacing ) + TTYFontWidth,
				 TTYCurrentLine * TTYLineHeight + TTYFontHeight );
}

void TTY_scrollUp( uint8_t lines )
{
	uint8_t scrollpixels = lines * TTYLineHeight;
	uint8_t scrollshiftbits = scrollpixels & 0x07;
	uint8_t scrollpages = scrollpixels >> 3;
	if ( scrollshiftbits == 0 )
	{
		if ( scrollpages == 0 )
		{
			return;
		}
		memcpy( STLcdBuffer,
			STLcdBuffer + scrollpages * LCD_PAGE_LEN,
			LCD_PAGE_LEN * ( LCD_TOTAL_VISIBLE_PAGES - scrollpages ) );
		memset( STLcdBuffer + LCD_PAGE_LEN * ( LCD_TOTAL_VISIBLE_PAGES - scrollpages ),
			0,
			scrollpages * LCD_PAGE_LEN );
	}
	else
	{
		if ( scrollpages < LCD_TOTAL_VISIBLE_PAGES - 1 )
		{
			for ( uint8_t destpage = LCD_TOTAL_VISIBLE_PAGES - 1,
				      srcpage = LCD_TOTAL_VISIBLE_PAGES - scrollpages - 1;
			      srcpage > 0;
			      destpage--, srcpage-- )
			{
				for ( uint8_t column = 0; column < LCD_PAGE_LEN; column++ )
				{
					STLcdBuffer[ destpage * LCD_PAGE_LEN + column ] =
						( STLcdBuffer[ srcpage * LCD_PAGE_LEN + column ] << scrollshiftbits ) |
						( STLcdBuffer[ ( srcpage - 1 ) * LCD_PAGE_LEN + column ] >> ( 8 - scrollshiftbits ) );
				}
			}
		}
		if ( scrollpages < LCD_TOTAL_VISIBLE_PAGES )
		{
			for ( uint8_t column = 0; column < LCD_PAGE_LEN; column++ )
			{ // last non-empty page
				STLcdBuffer[ scrollpages * LCD_PAGE_LEN + column ] = STLcdBuffer[ column ] << scrollshiftbits;
			}
		}
		if ( scrollpages > 0 ) // have empty pages
		{
			memset( STLcdBuffer,
				0,
				scrollpages * LCD_PAGE_LEN );
		}
	}
	STLcd_updateBoundingBox(0, 0, LCD_WIDTH, LCD_HEIGHT);
}

void TTY_newLine( void )
{
	if ( TTYCurrentLine > 0 )
	{
		TTYCurrentLine--;
		TTYCurrentColumn = 0;
	}
	else
	{
		TTY_scrollUp( 1 );
		TTYCurrentColumn = 0;
	}
}

void TTY_outputChar( uint8_t c )
{
	if ( !TTYInitialized ){
		TTY_initialize ( STLcdDefaultFont, STLcdDefaultFontSize, STLcdDefaultFontLength,
				 STLcdDefaultFontWidth, STLcdDefaultFontHeight,
				 1, 0 );
	}
	switch ( c )
	{
	case 0x8: // \b
		if ( TTYCurrentColumn )
		{
			TTYCurrentColumn--;
			TTY_drawGlyph( 0x20 );
		}
		break;
	case 0x9: // \t
		TTYCurrentColumn = ( TTYCurrentColumn & 0xfc ) + 4;
		if ( TTYCurrentColumn >= TTYColumns )
		{
			TTY_newLine();
		}
		break;
	case 0x0a: // \n
		TTY_newLine();
		break;
	case 0x0c: // \f
		STLcd_clear();
		TTYCurrentColumn = 0;
		TTYCurrentLine = TTYLines - 1;
		break;
	default:
		TTY_drawGlyph( c );
		TTYCurrentColumn++;
		if ( TTYCurrentColumn >= TTYColumns )
		{
			TTY_newLine();
		}
		break;
	}
}



// ----- CLI Command Functions -----

void cliFunc_lcdInit( char* args )
{
	LCD_initialize();
}

void cliFunc_lcdTest( char* args )
{
	// Write default image
	memcpy( STLcdBuffer, STLcdDefaultImage, LCD_PAGE_LEN * LCD_TOTAL_VISIBLE_PAGES );
	STLcd_updateBoundingBox( 0, 0, LCD_WIDTH, LCD_HEIGHT );
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

void cliFunc_lcdColor( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Colors
	uint16_t rgb[3]; // Red, Green, Blue

	// Parse integers from 3 arguments
	for ( uint8_t color = 0; color < 3; color++ )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Give up if not enough args given
		if ( *arg1Ptr == '\0' )
			return;

		// Convert argument to integer
		rgb[ color ] = numToInt( arg1Ptr );
	}

	// Set PWM channels
	FTM0_C0V = rgb[0];
	FTM0_C1V = rgb[1];
	FTM0_C2V = rgb[2];
}

void cliFunc_lcdDisp( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// First process page and starting address
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t page = numToInt( arg1Ptr );

	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t address = numToInt( arg1Ptr );
	uint8_t start = address;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		uint8_t value = numToInt( arg1Ptr );
		STLcdBuffer[ page * LCD_PAGE_LEN + address ] = value;
		address++;
	}
	STLcd_updateBoundingBox( start, page * 8, address, (page + 1) * 8 );
}

void cliFunc_lcdTextOut( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// First process page and starting address
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t x = numToInt( arg1Ptr );

	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t y = numToInt( arg1Ptr );

	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
		return;
	uint8_t value = numToInt( arg1Ptr );
	if ( value >= STLcdDefaultFontLength )
	{
		print( "The charactor: " );
		printHex( value );
		print( " is out of the boundary of the font(length ");
		printHex( STLcdDefaultFontLength );
		print( ")" );
		print( NL );
	}
	STLcd_drawBitmap( STLcdDefaultFont + value * STLcdDefaultFontSize, x, y,
			  STLcdDefaultFontWidth, STLcdDefaultFontHeight );
	STLcd_updateBoundingBox( x, y, x + STLcdDefaultFontWidth, y + STLcdDefaultFontHeight );
}

void cliFunc_ttyPrint( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		uint8_t value = numToInt( arg1Ptr );
		TTY_outputChar( value );
	}
}

void cliFunc_ttyScrollUp( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Stop processing args if no more are found
	if ( *arg1Ptr == '\0' )
	    return;

	uint8_t value = numToInt( arg1Ptr );
	TTY_scrollUp( value );
}
