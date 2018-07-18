/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2015 - 2017  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the RTT protocol and J-Link.                       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* conditions are met:                                                *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this list of conditions and the following disclaimer.    *
*                                                                    *
* o Redistributions in binary form must reproduce the above          *
*   copyright notice, this list of conditions and the following      *
*   disclaimer in the documentation and/or other materials provided  *
*   with the distribution.                                           *
*                                                                    *
* o Neither the name of SEGGER Microcontroller GmbH & Co. KG         *
*   nor the names of its contributors may be used to endorse or      *
*   promote products derived from this software without specific     *
*   prior written permission.                                        *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: V2.52a                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYSVIEW_Config_NoOS.c
Purpose : Sample setup configuration of SystemView without an OS.
Revision: $Rev: 7745 $
*/
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_Conf.h"

// SystemcoreClock can be used in most CMSIS compatible projects.
// In non-CMSIS projects define SYSVIEW_CPU_FREQ.
//extern unsigned int SystemCoreClock;
#define SystemCoreClock F_CPU

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#define SYSVIEW_APP_NAME        "Kira"
#define SYSVIEW_OS_NAME         "Kiibohd"
#define SYSVIEW_DEVICE_NAME     "sam4s8c"
#define SYSVIEW_CORE_NAME       "Cortex-M4"


// Frequency of the timestamp. Must match SEGGER_SYSVIEW_Conf.h
#define SYSVIEW_TIMESTAMP_FREQ  (SystemCoreClock)

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#define SYSVIEW_CPU_FREQ        (SystemCoreClock)

// The lowest RAM address used for IDs (pointers)
#define SYSVIEW_RAM_BASE        (0xfff8000)

// Define as 1 if the Cortex-M cycle counter is used as SystemView timestamp. Must match SEGGER_SYSVIEW_Conf.h
#ifndef   USE_CYCCNT_TIMESTAMP
  #define USE_CYCCNT_TIMESTAMP    1
#endif

// Define as 1 if the Cortex-M cycle counter is used and there might be no debugger attached while recording.
#ifndef   ENABLE_DWT_CYCCNT
  #define ENABLE_DWT_CYCCNT       (USE_CYCCNT_TIMESTAMP & SEGGER_SYSVIEW_POST_MORTEM_MODE)
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#define DEMCR                     (*(volatile unsigned long*) (0xE000EDFCuL))   // Debug Exception and Monitor Control Register
#define TRACEENA_BIT              (1uL << 24)                                   // Trace enable bit
#define DWT_CTRL                  (*(volatile unsigned long*) (0xE0001000uL))   // DWT Control Register
#define NOCYCCNT_BIT              (1uL << 25)                                   // Cycle counter support bit
#define CYCCNTENA_BIT             (1uL << 0)                                    // Cycle counter enable bit

/********************************************************************* 
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void) {
  SEGGER_SYSVIEW_SendSysDesc("N="SYSVIEW_APP_NAME",O="SYSVIEW_OS_NAME",D="SYSVIEW_DEVICE_NAME",C="SYSVIEW_CORE_NAME);
  SEGGER_SYSVIEW_SendSysDesc("I#1=Reset");
  SEGGER_SYSVIEW_SendSysDesc("I#2=NMI");
  SEGGER_SYSVIEW_SendSysDesc("I#3=HardFault");
  SEGGER_SYSVIEW_SendSysDesc("I#4=MemManage");
  //SEGGER_SYSVIEW_SendSysDesc("I#5=BusFault");
  SEGGER_SYSVIEW_SendSysDesc("I#6=UsageFault");
  SEGGER_SYSVIEW_SendSysDesc("I#11=SVC");
  SEGGER_SYSVIEW_SendSysDesc("I#12=DebugMon");
  SEGGER_SYSVIEW_SendSysDesc("I#14=PendSV");
  SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");

//#if defined(_sam_)
  SEGGER_SYSVIEW_SendSysDesc("I#16=SUPC");
  SEGGER_SYSVIEW_SendSysDesc("I#17=RSTC");
  SEGGER_SYSVIEW_SendSysDesc("I#18=RTC");
  SEGGER_SYSVIEW_SendSysDesc("I#19=RTT");
  SEGGER_SYSVIEW_SendSysDesc("I#20=WDT");
  SEGGER_SYSVIEW_SendSysDesc("I#21=PMC");
  SEGGER_SYSVIEW_SendSysDesc("I#22=EFC0");
  SEGGER_SYSVIEW_SendSysDesc("I#23=EFC1");
  SEGGER_SYSVIEW_SendSysDesc("I#24=UART0");
  SEGGER_SYSVIEW_SendSysDesc("I#25=UART1");
  SEGGER_SYSVIEW_SendSysDesc("I#27=PIOA");
  SEGGER_SYSVIEW_SendSysDesc("I#28=PIOB");
  SEGGER_SYSVIEW_SendSysDesc("I#29=PIOC");
  SEGGER_SYSVIEW_SendSysDesc("I#30=USART0");
  SEGGER_SYSVIEW_SendSysDesc("I#31=USART1");
  SEGGER_SYSVIEW_SendSysDesc("I#34=HSMCI");
  SEGGER_SYSVIEW_SendSysDesc("I#35=TWI0");
  SEGGER_SYSVIEW_SendSysDesc("I#36=TWI1");
  SEGGER_SYSVIEW_SendSysDesc("I#37=SPI");
  SEGGER_SYSVIEW_SendSysDesc("I#38=SSC");
  SEGGER_SYSVIEW_SendSysDesc("I#39=TC0");
  SEGGER_SYSVIEW_SendSysDesc("I#40=TC1");
  SEGGER_SYSVIEW_SendSysDesc("I#41=TC2");
  SEGGER_SYSVIEW_SendSysDesc("I#42=TC3");
  SEGGER_SYSVIEW_SendSysDesc("I#43=TC4");
  SEGGER_SYSVIEW_SendSysDesc("I#44=TC5");
  SEGGER_SYSVIEW_SendSysDesc("I#45=ADC");
  SEGGER_SYSVIEW_SendSysDesc("I#46=DACC");
  SEGGER_SYSVIEW_SendSysDesc("I#47=PWM");
  SEGGER_SYSVIEW_SendSysDesc("I#48=CRCCU");
  SEGGER_SYSVIEW_SendSysDesc("I#49=ACC");
  SEGGER_SYSVIEW_SendSysDesc("I#50=UDP");
//#endif
}


SEGGER_SYSVIEW_TASKINFO Task_cli_process = {
	.TaskID = TASK_CLI_PROCESS,
	.sName = "CLI_process",
	.Prio = 0,
	.StackBase = 0,
	.StackSize = 0
};

SEGGER_SYSVIEW_TASKINFO Task_scan_poll = {
	.TaskID = TASK_SCAN_POLL,
	.sName = "Scan_poll",
	.Prio = 0,
	.StackBase = 0,
	.StackSize = 0
};

SEGGER_SYSVIEW_TASKINFO Task_macro_poll = {
	.TaskID = TASK_MACRO_POLL,
	.sName = "Macro_poll",
	.Prio = 0,
	.StackBase = 0,
	.StackSize = 0
};

SEGGER_SYSVIEW_TASKINFO Task_output_poll = {
	.TaskID = TASK_OUTPUT_POLL,
	.sName = "Output_poll",
	.Prio = 0,
	.StackBase = 0,
	.StackSize = 0
};

SEGGER_SYSVIEW_TASKINFO Task_scan_periodic = {
	.TaskID = TASK_SCAN_PERIODIC,
	.sName = "Scan_periodic",
	.Prio = 1,
	.StackBase = 0,
	.StackSize = 0
};

SEGGER_SYSVIEW_TASKINFO Task_macro_periodic = {
	.TaskID = TASK_MACRO_PERIODIC,
	.sName = "Macro_periodic",
	.Prio = 1,
	.StackBase = 0,
	.StackSize = 0
};

SEGGER_SYSVIEW_TASKINFO Task_output_periodic = {
	.TaskID = TASK_OUTPUT_PERIODIC,
	.sName = "Output_periodic",
	.Prio = 1,
	.StackBase = 0,
	.StackSize = 0
};


void SendTaskList()
{
	SEGGER_SYSVIEW_SendTaskInfo(&Task_cli_process);
	SEGGER_SYSVIEW_SendTaskInfo(&Task_scan_poll);
	SEGGER_SYSVIEW_SendTaskInfo(&Task_macro_poll);
	SEGGER_SYSVIEW_SendTaskInfo(&Task_output_poll);
	SEGGER_SYSVIEW_SendTaskInfo(&Task_scan_periodic);
	SEGGER_SYSVIEW_SendTaskInfo(&Task_macro_periodic);
	SEGGER_SYSVIEW_SendTaskInfo(&Task_output_periodic);
}


/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void SEGGER_SYSVIEW_Conf(void) {
#if USE_CYCCNT_TIMESTAMP
#if ENABLE_DWT_CYCCNT
  //
  // If no debugger is connected, the DWT must be enabled by the application
  //
  if ((DEMCR & TRACEENA_BIT) == 0) {
    DEMCR |= TRACEENA_BIT;
  }
#endif
  //
  //  The cycle counter must be activated in order
  //  to use time related functions.
  //
  if ((DWT_CTRL & NOCYCCNT_BIT) == 0) {       // Cycle counter supported?
    if ((DWT_CTRL & CYCCNTENA_BIT) == 0) {    // Cycle counter not enabled?
      DWT_CTRL |= CYCCNTENA_BIT;              // Enable Cycle counter
    }
  }
#endif

	static SEGGER_SYSVIEW_OS_API api = {
		.pfGetTime = 0,
		.pfSendTaskList = SendTaskList
	};

  SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ, 
                      &api, _cbSendSystemDesc);
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

/*************************** End of file ****************************/
