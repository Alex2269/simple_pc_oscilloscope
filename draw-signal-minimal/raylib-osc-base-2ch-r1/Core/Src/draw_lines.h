
#ifndef __DRAW_LINES_H
#define __DRAW_LINES_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <raylib.h>

#include <stdint.h>
#include <math.h>
#include <stdlib.h>

extern uint16_t BACK_COLOR;
extern uint16_t POINT_COLOR;

void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color);
void display_signal(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color);

#ifdef __cplusplus
}
#endif

#endif /* __DRAW_LINES_H */
