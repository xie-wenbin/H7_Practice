#ifndef APPL_SDRAM_TEST_H
#define APPL_SDRAM_TEST_H
#include "bsp_fmc_sdram.h"

uint32_t bsp_TestExtSDRAM1(void);
void WriteSpeedTest(void);
void ReadSpeedTest(void);
void ReadWriteTest(void);

#endif /* APPL_SDRAM_TEST_H */
