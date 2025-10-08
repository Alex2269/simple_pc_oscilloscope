#include "draw_lines.h"

void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color)
{
  uint16_t t;
  int xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int incx, incy, uRow, uCol;
  delta_x = x2 - x1;
  delta_y = y2 - y1;
  uRow = x1;
  uCol = y1;

  if (delta_x > 0)
    incx = 1;
  else if (delta_x == 0)
    incx = 0;
  else {
    incx = -1;
    delta_x = -delta_x;
  }

  if (delta_y > 0)
    incy = 1;
  else if (delta_y == 0)
    incy = 0;
  else {
    incy = -1;
    delta_y = -delta_y;
  }

  if (delta_x > delta_y)
    distance = delta_x;
  else
    distance = delta_y;

  for (t = 0; t <= distance + 1; t++) {
    DrawPixel(uRow,uCol,color);
    // DrawRectangle(uRow, uCol, 1, 1, color);
    xerr += delta_x;
    yerr += delta_y;
    if (xerr > distance) {
      xerr -= distance;
      uRow += incx;
    }
    if (yerr > distance) {
      yerr -= distance;
      uCol += incy;
    }
  }
}

void display_signal(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color)
{
#define DOT_SIZE 2

  uint16_t t;
  int xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int incx, incy, uRow, uCol;
  delta_x = x2 - x1;
  delta_y = y2 - y1;
  uRow = x1;
  uCol = y1;

  if (delta_x > 0)
    incx = DOT_SIZE;
  else if (delta_x == 0)
    incx = 0;
  else {
    incx = -DOT_SIZE;
    delta_x = -delta_x;
  }

  if (delta_y > 0)
    incy = DOT_SIZE;
  else if (delta_y == 0)
    incy = 0;
  else {
    incy = -DOT_SIZE;
    delta_y = -delta_y;
  }

  if (delta_x > delta_y)
    distance = delta_x;
  else
    distance = delta_y;

  for (t = 0; t <= distance + 1; t += DOT_SIZE) {
    DrawPixel(uRow,uCol,color);
    // DrawRectangle(uRow, uCol, 1, 1, color);
    xerr += delta_x;
    yerr += delta_y;
    if (xerr > distance) {
      xerr -= distance;
      uRow += incx;
    }
    if (yerr > distance) {
      yerr -= distance;
      uCol += incy;
    }
  }
}
