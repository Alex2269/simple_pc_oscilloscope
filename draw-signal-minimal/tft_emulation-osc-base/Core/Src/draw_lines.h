
#ifndef __DRAW_LINES_H
#define __DRAW_LINES_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "vga_adafruit.h"
#include <math.h>
#include <stdlib.h>

#include "color.h"

extern uint16_t BACK_COLOR;
extern uint16_t POINT_COLOR;

#include "draw_lines.h"

void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
void display_signal(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* __DRAW_LINES_H */
