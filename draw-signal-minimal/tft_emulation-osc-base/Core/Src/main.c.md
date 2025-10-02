#include "vga_adafruit.h"
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

#include "color.h"
#include "fonts_all_includes.h"
#include "c_images_all_includes.h"
#include "lissajous.h"
#include "fill_polygon.h"
#include "DrawBezier.h"

#define WIDTH 800
#define HEIGHT 480

#define delay(a) usleep(a*1000)

void draw_dot_grid(void)
{
  // osMutexWait(Display_MutexHandle, portMAX_DELAY); // mutex on
  for(uint16_t y = 106;y<Display_Get_HEIGHT()-270;y+= 8)
    for(uint16_t x = 16; x<Display_Get_WIDTH()-15; x+=32) DrawRectangle(x, y, 2,2, CYAN);
  for(uint16_t y = 106;y<Display_Get_HEIGHT()-270;y+=32)
    for(uint16_t x = 16; x<Display_Get_WIDTH()-15; x+=8 ) DrawRectangle(x, y, 2,2, CYAN);
  // osMutexRelease(Display_MutexHandle); // mutex off
}

void draw_dynamic_grid(int16_t shiftx)
{
  if(shiftx < 12) return;
  #define ceil_x  33
    // osMutexAcquire(Display_MutexHandle, portMAX_DELAY);
  // osMutexWait(Display_MutexHandle, portMAX_DELAY); // mutex on
  for(uint16_t y = 32;y<Display_Get_HEIGHT()-31;y+= 8)
    for(uint16_t x = shiftx; x<ceil_x+shiftx; x+=32) DrawRectangle(x, y, 1,1, CYAN);
  for(uint16_t y = 32;y<Display_Get_HEIGHT()-31;y+=32)
    for(uint16_t x = shiftx; x<ceil_x+shiftx; x+=8 ) DrawRectangle(x, y, 1,1, CYAN);
  // osMutexRelease(Display_MutexHandle); // mutex off
}

#include "rs232.h"
#include "stdlib.h"
unsigned char buffer[4];

uint16_t i = 0;
int main(void){
    i++;
    gfx_open(WIDTH,HEIGHT,"tft emulation");
    Display_Set_WIDTH(WIDTH);
    Display_Set_HEIGHT(HEIGHT);

    DrawRectangle(0, 0, Display_Get_WIDTH(),Display_Get_HEIGHT(), GRAY50);

    fillRoundRect(32, 32, Display_Get_WIDTH()-64, Display_Get_HEIGHT()-64, 8, BLACK);
    // fillRoundRect(32, 32, Display_Get_WIDTH()-64, Display_Get_HEIGHT()-64, 8, DARKSLATEGRAY);
    // fillRoundRect(8, 120, Display_Get_WIDTH()-15, Display_Get_HEIGHT()-240, 6, DARKBLUE);

    gfx_color(128,127,255);

    // draw_line(20,20,20+100,20+10, 0x00ff00);
    // drawCircle(100, 100, 50, 0x0000ff);
    // draw_dot_grid();

    for(int16_t i=8;i<Display_Get_WIDTH()-8;i++)
    {
        if( !(i % 32) ) draw_dynamic_grid(i-33);
    }
    if(i>Display_Get_WIDTH()) i=0;

    int val ;

    // impuls1 = val % 256;
    // impuls2 = val / 256;

    int  cport_nr=24,  //cat /dev/ttyACM0;
         bdrate=1000000;
    char mode[]={'8','N','1',0};

    RS232_OpenComport(cport_nr, bdrate, mode, 0);

    while(1){

    for(int16_t i=8;i<Display_Get_WIDTH()-8;i++)
    {
      RS232_PollComport(24, buffer, 4);
      // SendBuf(0, '\0', 5);

      char* endptr = NULL;
      val = strtol(buffer, &endptr, 10);
      // val = (int)atoi(buffer);

      for(uint8_t i=0;i<4;i++) buffer[i]='\0';

      if(val)
        printf("%04d\n", val);

      if(val>32)
        DrawRectangle(i,HEIGHT-val/9, 1,1, YELLOW);

      if(!(i % 32))
        draw_dynamic_grid(i-33);
      usleep(3e2);
    }
      gfx_flush(); //Update the graphics
    }
}

