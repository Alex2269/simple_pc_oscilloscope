#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

#include "vga_adafruit.h"
#include "gfx.h"
#include "tft.h"
#include "pixel.h"
#include "osc.h"

#include "rs232.h"

// #include "draw_lines.h"
// #include "color.h"

#define WIDTH 800
#define HEIGHT 480

#define delay(a) usleep(a*1000)

int main(void){

    gfx_open(WIDTH,HEIGHT,"tft emulation");
    Display_Set_WIDTH(WIDTH);
    Display_Set_HEIGHT(HEIGHT);

    // DrawRectangle(0, 0, Display_Get_WIDTH(),Display_Get_HEIGHT(), GRAY50);
    // fillRoundRect(32, 32, Display_Get_WIDTH()-64, Display_Get_HEIGHT()-64, 8, DARKSLATEGRAY);
    // tft_draw_dot_line(32, 32, Display_Get_WIDTH()-32, Display_Get_HEIGHT()-32, SLATEGRAY);

    gfx_color(128,127,255);

    oscil_init();

    while(1) {
      oscil_run();
    }
}

