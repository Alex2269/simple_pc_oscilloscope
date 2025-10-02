#ifndef __gfx_H
#define __gfx_H

/* Include Library */
#include "stdint.h"
#include "stdbool.h"
// #include "ftoa_impl.h"

void gfx_open(int width, int height, const char *title);
void gfx_flush(void);

#endif // __gfx_H


