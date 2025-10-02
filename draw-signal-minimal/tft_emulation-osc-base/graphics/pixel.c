#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "pixel.h"
#include "color.h"

extern Display *gfx_display;
extern Window gfx_window;
extern GC gfx_gc;
extern int gfx_fast_color_mode;

void DrawPixel(uint16_t x, uint16_t y, uint32_t color)
{
  uint8_t r,g,b;
  r = (uint8_t)(color >> 16);
  g = (uint8_t)(color >> 8);
  b = (uint8_t)(color );
  gfx_color(r, g, b);
  XDrawPoint(gfx_display,gfx_window,gfx_gc,x,y);
}

/* Change the current drawing color. */
void gfx_color( int r, int g, int b )
{
  XColor color;
  if(gfx_fast_color_mode) 
  {
    /* If this is a truecolor display, we can just pick the color directly. */
    color.pixel = ((b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16) );
  }
  XSetForeground(gfx_display, gfx_gc, color.pixel);
}
