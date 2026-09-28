#ifndef OLED_H
#define OLED_H

#include "stm32f1xx_hal.h"

#define OLED_I2C_ADDRESS   0x3C
#define OLED_WIDTH         128
#define OLED_HEIGHT        64

void oled_init(void);
void oled_clear(void);
void oled_show_text(uint8_t col, uint8_t page, const char *text);

#endif