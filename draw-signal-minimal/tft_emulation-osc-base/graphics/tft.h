#ifndef __tft___H_
#define __tft___H_

#include "stdint.h"
#include "color.h"

 /* Set here your display resolution. */

extern uint16_t DISPLAY_WIDTH ; // = 800;
extern uint16_t DISPLAY_HEIGHT ; // = 480;

#define Display_FAIL 0
#define Display_OK   1

/* *** Function prototypes. *** */
uint16_t Display_Get_WIDTH(void);
uint16_t Display_Get_HEIGHT(void);
void Display_Set_WIDTH(uint16_t x);
void Display_Set_HEIGHT(uint16_t y);
void Display_Set_rotation(uint8_t r);
void Display_Init(uint16_t r);
void Display_Set_backlight(uint8_t percent);
void Display_WindowSet(unsigned int s_x,unsigned int e_x,unsigned int s_y,unsigned int e_y);
void Display_PSet(uint16_t x, uint16_t y, uint32_t c);

#endif /* __tft___H */

/* [] END OF FILE */

