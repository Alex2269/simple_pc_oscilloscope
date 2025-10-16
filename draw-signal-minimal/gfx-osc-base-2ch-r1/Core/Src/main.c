#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

#include "gfx.h"
#include "display.h"
#include "osc.h"

#include "rs232.h"

#define WIDTH 800
#define HEIGHT 480

#define delay(a) usleep(a*1000)

int main(void){

    gfx_open(WIDTH,HEIGHT,"tft emulation");
    Display_Set_WIDTH(WIDTH);
    Display_Set_HEIGHT(HEIGHT);

    gfx_color(128,127,255);

    oscil_init();

    while(1) {
      oscil_run();
      gfx_flush();
    }
    return 0;
}

