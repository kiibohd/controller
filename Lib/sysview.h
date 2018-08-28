/* Copyright (C) 2018 by Jacob Alexander
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

#pragma once

#ifdef SYSTEMVIEW_ENABLED
#include "SEGGER_SYSVIEW.h"

#else
#define SEGGER_SYSVIEW_RecordVoid()
#define SEGGER_SYSVIEW_RecordU32(x)
#define SEGGER_SYSVIEW_RecordString(x, y)
#define SEGGER_SYSVIEW_RecordSystime()
#define SEGGER_SYSVIEW_RecordEnterISR()
#define SEGGER_SYSVIEW_RecordExitISR()
#define SEGGER_SYSVIEW_RecordExitISRToScheduler()
#define SEGGER_SYSVIEW_RecordEnterTimer(x)
#define SEGGER_SYSVIEW_RecordExitTimer()
#define SEGGER_SYSVIEW_RecordEndCall(x)
#define SEGGER_SYSVIEW_RecordEndCallU32(x, y)

#define SEGGER_SYSVIEW_OnIdle()
#define SEGGER_SYSVIEW_OnTaskCreate(x)
#define SEGGER_SYSVIEW_OnTaskTerminate(x)
#define SEGGER_SYSVIEW_OnTaskStartExec(x)
#define SEGGER_SYSVIEW_OnTaskStopExec()
#define SEGGER_SYSVIEW_OnTaskStartReady(x)
#define SEGGER_SYSVIEW_OnTaskStopReady(x, y)
#define SEGGER_SYSVIEW_OnUserStart(x)
#define SEGGER_SYSVIEW_OnUserStop(x)

#endif
