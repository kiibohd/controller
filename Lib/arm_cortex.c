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

#if defined(_bootloader_)
#include <debug.h>
#else
#include <print.h>
#endif

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

void cm4_init() {
	SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
	SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk;

#if defined(DEBUG)
	// Can give more precise busfault addresses
	disable_write_buffering();
#endif

#if defined(JLINK)
	// Break on reset or failed interrupt
	CoreDebug->DEMCR |= CoreDebug_DEMCR_VC_INTERR_Msk | CoreDebug_DEMCR_VC_CORERESET_Msk;

	// Initialize micro-trace buffer
	//REG_MTB_POSITION = ((uint32_t) (mtb - REG_MTB_BASE)) & 0xFFFFFFF8;
	//REG_MTB_FLOW = ((uint32_t) mtb + TRACE_BUFFER_SIZE * sizeof(uint32_t)) & 0xFFFFFFF8;
	//REG_MTB_MASTER = 0x80000000 + 6;
#endif
}

// Causes more BusFaults to be precise (good for debugging)
void disable_write_buffering() {
	SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
}

Cortex_IRQ get_current_isr()
{
	uint32_t ipsr;
	__asm volatile ("mrs %0, ipsr" : "=r" (ipsr) );

	return (Cortex_IRQ) (ipsr & 0xFF);
}

void hardfault_handler( uint32_t *faultStackedAddress )
{
	/* These are volatile to try and prevent the compiler/linker optimising them
	 * away as when debugging as the variables never actually get used. */
	volatile StackFrame_t stack __attribute__((unused));
	volatile   CFSR_t _CFSR __attribute__((unused));
	volatile   HFSR_t _HFSR __attribute__((unused));
	volatile   DFSR_t _DFSR __attribute__((unused));
	volatile uint32_t _AFSR __attribute__((unused));
	volatile uint32_t _BFAR __attribute__((unused));
	volatile uint32_t _MMAR __attribute__((unused));

	stack = *(StackFrame_t*) faultStackedAddress; // Previous stack frame
	_CFSR = *CFSR; // Configurable Fault Status Register
	_HFSR = *HFSR; // Hard Fault Status Register
	_DFSR = *DFSR; // Debug Fault Status Register
	_AFSR = SCB->AFSR; // Auxiliary Fault Status Register ( Vendor Specific )
	_MMAR = SCB->MMFAR; // MemManage Fault Address Register
	_BFAR = SCB->BFAR; // Bus Fault Address Register

	/* gdb:
	 *      set output-radix 16
	 *      info locals
	 *      x/i _MMAR
	 *      x/i _BFAR
	 *      x/i stack.pc
	 */
	fault_isr();
}

void memfault_handler( uint32_t *faultStackedAddress ) {
	volatile StackFrame_t frame = *(StackFrame_t*) faultStackedAddress;
	volatile MMFSR_t MMFSR = CFSR->MMFSR;
	uint32_t MMFAR = SCB->MMFAR;
	print("MemFault!" NL);

	if (MMFSR.MMARVALID) {
		print(" Attempt to access address ");
		printHex32(MMFAR);
		print(NL);
	}
	if (MMFSR.DACCVIOL) {
		print(" Operation not permitted" NL);
		print(" PC = ");
		printHex32(frame.pc);
		print(NL);
	}
	if (MMFSR.IACCVIOL) {
		print(" Non-executable region" NL);
		print(" PC = ");
		printHex32(frame.pc);
		print(NL);
	}
	if (MMFSR.MSTKERR) {
		print(" Stacking error" NL);
	}

	fault_isr();
}

void busfault_handler( uint32_t *faultStackedAddress ) {
	volatile StackFrame_t frame = *(StackFrame_t*) faultStackedAddress;
	volatile BFSR_t BFSR = CFSR->BFSR;
	uint32_t BFAR = SCB->BFAR;
	print("BusFault!" NL);

	if (BFSR.BFARVALID) {
		print(" Attempt to access address ");
		printHex32(BFAR);
		print(NL);
	}
	if (BFSR.PRECISERR) {
		print(" PC = ");
		printHex32(frame.pc);
		print(NL);
	}
	if (BFSR.IBUSERR) {
		print(" Error fetching instruction" NL);
	}
	if (BFSR.STKERR) {
		print(" Stacking error" NL);
	}

	fault_isr();
}

void usagefault_handler( uint32_t *faultStackedAddress ) {
	volatile StackFrame_t frame = *(StackFrame_t*) faultStackedAddress;
	volatile UFSR_t UFSR = CFSR->UFSR;
	print("UsageFault!" NL);

	if (UFSR.DIVBYZERO) {
		print(" Divide by 0" NL);
		print(" PC = ");
		printHex32(frame.pc);
		print(NL);
	}
	if (UFSR.UNDEFINSTR) {
		print(" Undefined instruction" NL);
	}

	fault_isr();
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
	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
	MPU->CTRL = MPU_CTRL_ENABLE_Msk;
}
