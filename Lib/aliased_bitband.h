
#ifndef __aliased_bitband_h
#define __aliased_bitband_h


// Aliased Regions for single bit (0th) register access

// Chapter 4: Memory Map (Table 4-1)



// TODO
// - Not all tested, and not all sections added



// 0x2200 0000 - 0x23FF FFFF - Aliased to SRAM_U bitband
// TODO



// 0x4200 0000 - 0x43FF FFFF - Aliased to AIPS and GPIO bitband
#define GPIO_BITBAND_ADDR(reg, bit) (((uint32_t)&(reg) - 0x40000000) * 32 + (bit) * 4 + 0x42000000)
#define GPIO_BITBAND_PTR(reg, bit) ((uint32_t *)GPIO_BITBAND_ADDR((reg), (bit)))

// XXX - Only MODREG is tested to work...
#define GPIO_BITBAND_OUTREG(reg, bit) *((uint32_t *)GPIO_BITBAND_ADDR((reg), (bit)) +   0)
#define GPIO_BITBAND_SETREG(reg, bit) *((uint32_t *)GPIO_BITBAND_ADDR((reg), (bit)) +  32)
#define GPIO_BITBAND_CLRREG(reg, bit) *((uint32_t *)GPIO_BITBAND_ADDR((reg), (bit)) +  64)
#define GPIO_BITBAND_TOGREG(reg, bit) *((uint32_t *)GPIO_BITBAND_ADDR((reg), (bit)) +  96)
#define GPIO_BITBAND_INPREG(reg, bit) *((uint32_t *)GPIO_BITBAND_ADDR((reg), (bit)) + 128)
#define GPIO_BITBAND_MODREG(reg, bit) *((uint32_t *)GPIO_BITBAND_ADDR((reg), (bit)) + 160)



#endif

