/* Copyright (C) 2018 by Rowan Decker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**< Interrupt Number Definition */
typedef enum IRQn
{
/******  Cortex-M4 Processor Exceptions Numbers ******************************/
  NonMaskableInt_IRQn   = -14, /**<  2 Non Maskable Interrupt                */
  MemoryManagement_IRQn = -12, /**<  4 Cortex-M4 Memory Management Interrupt */
  BusFault_IRQn         = -11, /**<  5 Cortex-M4 Bus Fault Interrupt         */
  UsageFault_IRQn       = -10, /**<  6 Cortex-M4 Usage Fault Interrupt       */
  SVCall_IRQn           = -5,  /**< 11 Cortex-M4 SV Call Interrupt           */
  DebugMonitor_IRQn     = -4,  /**< 12 Cortex-M4 Debug Monitor Interrupt     */
  PendSV_IRQn           = -2,  /**< 14 Cortex-M4 Pend SV Interrupt           */
  SysTick_IRQn          = -1,  /**< 15 Cortex-M4 System Tick Interrupt       */
} IRQn_Type;

/**
 * \brief Configuration of the Cortex-M4 Processor and Core Peripherals
 */

#define __CM4_REV              0x0001 /**< SAM4S8B core revision number ([15:8] revision number, [7:0] patch number) */
#define __MPU_PRESENT          1      /**< SAM4S8B does provide a MPU */
#define __FPU_PRESENT          0      /**< SAM4S8B does not provide a FPU */
#define __NVIC_PRIO_BITS       4      /**< SAM4S8B uses 4 Bits for the Priority Levels */
#define __Vendor_SysTickConfig 0      /**< Set to 1 if different SysTick Config is used */
#include "arm_cortex.h"

Cortex_IRQ get_current_isr()
{
	uint32_t ipsr;
	__asm volatile ("mrs %0, ipsr" : "=r" (ipsr) );

	return (Cortex_IRQ) (ipsr & 0xFF);
}

void __attribute__((naked)) debug_isr ( void ) {
	__asm volatile(
		"tst lr, #4\t\n"
		"ite eq\t\n"
		"mrseq r0, msp\t\n"
		"mrsne r0, psp\t\n"
		"b read_stacked_fault_frame\t\n"
		: // no output
		: // no input
		: "r0" // clobber
	);
}

void read_stacked_fault_frame( uint32_t *faultStackedAddress )
{
	/* These are volatile to try and prevent the compiler/linker optimising them
	 * away as the variables never actually get used.  If the debugger won't show the
	 * values of the variables, make them global my moving their declaration outside
	 * of this function. */
	volatile uint32_t    r0 __attribute__((unused));
	volatile uint32_t    r1 __attribute__((unused));
	volatile uint32_t    r2 __attribute__((unused));
	volatile uint32_t    r3 __attribute__((unused));
	volatile uint32_t   r12 __attribute__((unused));
	volatile uint32_t    lr __attribute__((unused));
	volatile uint32_t    pc __attribute__((unused));
	volatile uint32_t   psr __attribute__((unused));
	volatile   CFSR_t _CFSR __attribute__((unused));
	volatile   HFSR_t _HFSR __attribute__((unused));
	volatile   DFSR_t _DFSR __attribute__((unused));
	volatile uint32_t _AFSR __attribute__((unused));
	volatile uint32_t _BFAR __attribute__((unused));
	volatile uint32_t _MMAR __attribute__((unused));

	r0  = faultStackedAddress[ 0 ];
	r1  = faultStackedAddress[ 1 ];
	r2  = faultStackedAddress[ 2 ];
	r3  = faultStackedAddress[ 3 ];
	r12 = faultStackedAddress[ 4 ];
	lr  = faultStackedAddress[ 5 ];
	pc  = faultStackedAddress[ 6 ];
	psr = faultStackedAddress[ 7 ];

	// Configurable Fault Status Register
	// Consists of MMSR, BFSR and UFSR
	_CFSR = (*((volatile CFSR_t *)(0xE000ED28))) ;

	// Hard Fault Status Register
	_HFSR = (*((volatile HFSR_t *)(0xE000ED2C))) ;

	// Debug Fault Status Register
	_DFSR = (*((volatile DFSR_t *)(0xE000ED30))) ;

	// Auxiliary Fault Status Register ( Vendor Specific )
	_AFSR = (*((volatile unsigned long *)(0xE000ED3C))) ;

	// Fault Address Registers. These may not contain valid values.
	// Check BFARVALID/MMARVALID to see if they are valid values

	// MemManage Fault Address Register
	_MMAR = (*((volatile unsigned long *)(0xE000ED34))) ;
	// Bus Fault Address Register
	_BFAR = (*((volatile unsigned long *)(0xE000ED38))) ;

	/* gdb:
	 *      set output-radix 16
	 *      info locals
	 *      x/i _MMAR
	 *      x/i _BFAR
	 *      x/i pc
	 */
	__asm volatile("BKPT #01");
	for( ;; );
}

// Causes more BusFaults to be precise (good for debugging)
void disable_write_buffering() {
	SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
}

void mpu_setup_region(uint8_t region, uint32_t *start, uint32_t *end, uint32_t attr) {
	MPU->RBAR = (( (uint32_t)start << MPU_RBAR_ADDR_Pos ) & MPU_RBAR_ADDR_Msk) |
		((region << MPU_RBAR_REGION_Pos) & MPU_RBAR_REGION_Msk) |
		((1 << MPU_RBAR_VALID_Pos) & MPU_RBAR_VALID_Msk);

	uint8_t size_pow = 32 - __CLZ(end - start) - 1;
	MPU->RASR = ((size_pow << MPU_RASR_SIZE_Pos) & MPU_RASR_SIZE_Msk) |
		((1 << MPU_RASR_ENABLE_Pos) & MPU_RASR_ENABLE_Msk) |
	      attr;
}

void mpu_enable() {
	//MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
	MPU->CTRL = MPU_CTRL_ENABLE_Msk;
}
