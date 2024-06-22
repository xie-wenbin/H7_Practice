#ifndef APPL_LCD_TEST_H_
#define APPL_LCD_TEST_H_

#include "bsp_lcd_utils.h"
#include "bsp_lcd.h"

void TouchScreenDemo1(void);
#if (USE_TS_MULTI_TOUCH == 1)
void Touchscreen_demo2(void);
#endif

#endif /* APPL_LCD_TEST_H_ */
