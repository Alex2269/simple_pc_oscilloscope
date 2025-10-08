#ifndef __pix_H
#define __pix_H

/* Include Library */
#include "stdint.h"
#include "stdbool.h"

void DrawPixel(uint16_t x, uint16_t y, uint32_t color);
void gfx_color(int r, int g, int b);

#endif // __pix_H


