// file draw_grid.h

#ifndef DRAW_GRID_H_
#define DRAW_GRID_H_

#include "vga_adafruit.h"
#include "color.h"
#include <stdint.h>

// --- Прототипи функцій ---
void draw_grid_layer(int startX, int endX, int stepX,
                     int startY, int endY, int stepY,
                     uint32_t color);

void draw_grid(int screenWidth, int screenHeight);


#endif // DRAW_GRID_H_

