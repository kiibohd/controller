/* Copyright (C) 2017-2018 by Jacob Alexander
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


// ----- Defined Guide -----
//
// _kinetis_     - Kinetis MCU
// _kinetis_k_   - Kinetis K-Series MCU
// _kinetis_k2x_ - Kinetis K2x-Series MCU
// _kinetis_k6x_ - Kinetis K6x-Series MCU
// _kinetis_k20_ - Kinetis K20-Series MCU
// _kinetis_k22_ - Kinetis K22-Series MCU
// _kinetis_k64_ - Kinetis K64-Series MCU
// _kinetis_k66_ - Kinetis K66-Series MCU
// _kinetis_fpu_ - Kinetis MCU with FPU
//
// _sam_      - SAM MCU
// _sam4s_    - SAM 4s    MCU
// _sam4s2_   - SAM 4s2   MCU
// _sam4s4_   - SAM 4s4   MCU
// _sam4s8_   - SAM 4s8   MCU
// _sam4s16_  - SAM 4s16  MCU
// _sam4sa16_ - SAM 4sa16 MCU
// _sam4sd16_ - SAM 4sd16 MCU
// _sam4sd32_ - SAM 4sd32 MCU
// _sam4s_a_  - SAM 4s 48-pin  MCU
// _sam4s_b_  - SAM 4s 64-pin  MCU
// _sam4s_c_  - SAM 4s 100-pin MCU
//
// _nrf_        - nRF      BLE MCU
// _nrf5_       - nRF5     BLE MCU
// _nrf52_      - nRF52    BLE MCU
// _nrf528_     - nRF528   BLE MCU
// _nrf52810_   - nRF52810 BLE MCU
// _nrf52832_   - nRF52832 BLE MCU
// _nrf52832_a_ - nRF52832 BLE MCU 512 kB
// _nrf52832_b_ - nRF52832 BLE MCU 256 kB
// _nrf52840_   - nRF52840 BLE MCU
//
// _kii_v1_ - Kiibohd Firmware Layout v1 - First Keyboard: Infinity 60%
// _kii_v2_ - Kiibohd Firmware Layout v2 - First Keyboard: Infinity Ergodox
// _kii_v3_ - Kiibohd Firmware Layout v3 - First Keyboard: TBD
//
// _teensy_          - PJRC Teensy
// _teensy_2_        - PJRC Teensy 2 Series
// _teensy_3_        - PJRC Teensy 3 Series
// _teensy_3_0_      - PJRC Teensy 3.0
// _teensy_3_0__3_1  - PJRC Teensy 3.0+3.1 (+3.2)
// _teensy_3_1_      - PJRC Teensy 3.1 (+3.2)
// _teensy_3_5_      - PJRC Teensy 3.5
// _teensy_3_5__3_6_ - PJRC Teensy 3.5+3.6
// _teensy_3_6_      - PJRC Teensy 3.6



// ----- Includes -----

#pragma once

// - Kinetis ARM MCUs -

// mk20dx128vlf5
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_)
	#define _kinetis_ 1
	#define _kinetis_k_ 1
	#define _kinetis_k2x_ 1
	#define _kinetis_k20_ 1
	#define _kii_v1_ 1
#endif

// mk20dx128vlh7
#if defined(_mk20dx128vlh7_)
	#define _kinetis_ 1
	#define _kinetis_k_ 1
	#define _kinetis_k2x_ 1
	#define _kinetis_k20_ 1
	#define _kii_v2_ 1
#endif

// mk20dx256vlh7
#if defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
	#define _kinetis_ 1
	#define _kinetis_k_ 1
	#define _kinetis_k2x_ 1
	#define _kinetis_k20_ 1
	#define _kii_v2_ 1
#endif

// mk22fx512avlh12
#if defined(_mk22fx512avlh12_)
	#define _kinetis_ 1
	#define _kinetis_k_ 1
	#define _kinetis_k2x_ 1
	#define _kinetis_k22_ 1
	#define _kinetis_fpu_ 1
	#define _kii_v2_ 1
#endif

// mk64fx512vmd12
#if defined(_mk64fx512_)
	#define _kinetis_ 1
	#define _kinetis_k_ 1
	#define _kinetis_k6x_ 1
	#define _kinetis_k64_ 1
	#define _kinetis_fpu_ 1
#endif

// mk66fx1m0vmd18
#if defined(_mk66fx1m0_)
	#define _kinetis_ 1
	#define _kinetis_k_ 1
	#define _kinetis_k6x_ 1
	#define _kinetis_k64_ 1
	#define _kinetis_fpu_ 1
#endif



// - SAM ARM MCUs -

// sam4s2a (128 kB) 48-pin
#if defined(_sam4s2a_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_a_ 1
	#define _sam4s2_ 1
	#define _kii_v3_ 1
#endif

// sam4s2b (128 kB) 64-pin
#if defined(_sam4s2b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_b_ 1
	#define _sam4s2_ 1
	#define _kii_v3_ 1
#endif

// sam4s2c (128 kB) 100-pin
#if defined(_sam4s2c_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_c_ 1
	#define _sam4s2_ 1
	#define _kii_v3_ 1
#endif

// sam4s4a (256 kB) 48-pin
#if defined(_sam4s4a_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_a_ 1
	#define _sam4s4_ 1
	#define _kii_v3_ 1
#endif

// sam4s4b (256 kB) 64-pin
#if defined(_sam4s4b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_b_ 1
	#define _sam4s4_ 1
	#define _kii_v3_ 1
#endif

// sam4s4c (256 kB) 100-pin
#if defined(_sam4s4c_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_c_ 1
	#define _sam4s4_ 1
	#define _kii_v3_ 1
#endif

// sam4s8b (512 kB) 64-pin
#if defined(_sam4s8b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_b_ 1
	#define _sam4s8_ 1
	#define _kii_v3_ 1
#endif

// sam4s8c (512 kB) 100-pin
#if defined(_sam4s8c_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_c_ 1
	#define _sam4s8_ 1
	#define _kii_v3_ 1
#endif

// sam4s16b (1028 kB) 64-pin
#if defined(_sam4s16b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_b_ 1
	#define _sam4s16_ 1
	#define _kii_v3_ 1
#endif

// sam4s16c (1028 kB) 100-pin
#if defined(_sam4s16b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_c_ 1
	#define _sam4s16_ 1
	#define _kii_v3_ 1
#endif

// sam4sa16b (sam4s16b /w HCache) (1024 kB) 64-pin
#if defined(_sam4sa16b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_b_ 1
	#define _sam4sa_ 1
	#define _sam4sa16_ 1
	#define _sam_hcache_ 1
	#define _kii_v3_ 1
#endif

// sam4sa16c (sam4s16b /w HCache) (1024 kB) 100-pin
#if defined(_sam4sa16b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_c_ 1
	#define _sam4sa_ 1
	#define _sam4sa16_ 1
	#define _sam_hcache_ 1
	#define _kii_v3_ 1
#endif

// sam4sd16b (HCache) (2 x 512 kB) 64-pin
#if defined(_sam4sd16b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_b_ 1
	#define _sam4sd_ 1
	#define _sam4sd16_ 1
	#define _sam_hcache_ 1
	#define _kii_v3_ 1
#endif

// sam4sd16c (HCache) (2 x 512 kB) 100-pin
#if defined(_sam4sd16b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_c_ 1
	#define _sam4sd_ 1
	#define _sam4sd16_ 1
	#define _sam_hcache_ 1
	#define _kii_v3_ 1
#endif

// sam4sd32b (HCache) (2 x 1024 kB) 64-pin
#if defined(_sam4sd32b_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_b_ 1
	#define _sam4sd_ 1
	#define _sam4sd32_ 1
	#define _sam_hcache_ 1
	#define _kii_v3_ 1
#endif

// sam4sd32c (HCache) (2 x 1024 kB) 100-pin
#if defined(_sam4sd32c_)
	#define _sam_ 1
	#define _sam4s_ 1
	#define _sam4s_c_ 1
	#define _sam4sd_ 1
	#define _sam4sd32_ 1
	#define _sam_hcache_ 1
	#define _kii_v3_ 1
#endif



// - nRF BLE ARM MCUs -

// nRF52810-QCAA (192 kB) 32-pin
#if defined(_nrf52810_qcaa_)
	#define _nrf_ 1
	#define _nrf5_ 1
	#define _nrf52_ 1
	#define _nrf528_ 1
	#define _nrf52810_ 1
#endif

// nRF52810-QFAA (192 kB) 48-pin
#if defined(_nrf52810_qfaa_)
	#define _nrf_ 1
	#define _nrf5_ 1
	#define _nrf52_ 1
	#define _nrf528_ 1
	#define _nrf52810_ 1
#endif

// nRF52832-QFAA (512 kB) 48-pin
#if defined(_nrf52832_qfaa_)
	#define _nrf_ 1
	#define _nrf5_ 1
	#define _nrf52_ 1
	#define _nrf528_ 1
	#define _nrf52832_ 1
	#define _nrf52832_a_ 1
#endif

// nRF52832-QFAB (256 kB) 48-pin
#if defined(_nrf52832_qfab_)
	#define _nrf_ 1
	#define _nrf5_ 1
	#define _nrf52_ 1
	#define _nrf528_ 1
	#define _nrf52832_ 1
	#define _nrf52832_b_ 1
#endif

// nRF52832-CIAA (512 kB) 50-pin
#if defined(_nrf52832_ciaa_)
	#define _nrf_ 1
	#define _nrf5_ 1
	#define _nrf52_ 1
	#define _nrf528_ 1
	#define _nrf52832_ 1
	#define _nrf52832_a_ 1
#endif

// nRF52840-QIAA (1024 kB) 73-pin
#if defined(_nrf52840_qiaa_)
	#define _nrf_ 1
	#define _nrf5_ 1
	#define _nrf52_ 1
	#define _nrf528_ 1
	#define _nrf52840_ 1
#endif



// - AVR MCUs -

// at90usb1286
#if defined(_at90usb1286_)
	#define _avr_at_ 1
#endif

// atmega32u4
#if defined(_atmega32u4_)
	#define _avr_at_ 1
#endif



// - Teensy -

// Teensy 2.0 - atmega32u4
#if defined(_atmega32u4_)
	#define _teensy_ 1
	#define _teensy_2_ 1
#endif

// Teensy 2.0++ - at90usb1286
#if defined(_at90usb1286_)
	#define _teensy_ 1
	#define _teensy_2_ 1
#endif

// Teensy 3.0 - mk20dx128vlf5
#if defined(_mk20dx128_)
	#define _teensy_ 1
	#define _teensy_3_ 1
	#define _teensy_3_0_ 1
	#define _teensy_3_0__3_1 1
#endif

// Teensy 3.1/3.2 - mk20dx256vlh7
#if defined(_mk20dx256_)
	#define _teensy_ 1
	#define _teensy_3_ 1
	#define _teensy_3_1_ 1
	#define _teensy_3_0__3_1 1
#endif

// Teensy 3.5 - mk64fx512vmd12
#if defined(_mk64fx512_)
	#define _teensy_ 1
	#define _teensy_3_ 1
	#define _teensy_3_5_ 1
	#define _teensy_3_5__3_6_ 1
#endif

// Teensy 3.6 - mk66fx1m0vmd18
#if defined(_mk66fx1m0_)
	#define _teensy_ 1
	#define _teensy_3_ 1
	#define _teensy_3_6_ 1
	#define _teensy_3_5__3_6_ 1
#endif

