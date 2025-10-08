#include "stdio.h"
#include "stdbool.h"
// #include "main.h"
#include "tft.h"

uint16_t DISPLAY_WIDTH;
uint16_t DISPLAY_HEIGHT;

uint16_t Display_Get_WIDTH(void)
{
  return DISPLAY_WIDTH;
}

uint16_t Display_Get_HEIGHT(void)
{
  return DISPLAY_HEIGHT;
}

void Display_Set_WIDTH(uint16_t x)
{
  DISPLAY_WIDTH = x;
}

void Display_Set_HEIGHT(uint16_t y)
{
  DISPLAY_HEIGHT = y;
}

void Display_Set_rotation(uint8_t r)
{}

void Display_Init(uint16_t r)
{}

void Display_Set_backlight(uint8_t percent)
{}

void Display_WindowSet(unsigned int s_x,unsigned int e_x,unsigned int s_y,unsigned int e_y)
{}

void Display_PSet(uint16_t x, uint16_t y, uint32_t c)
{}

/* [] END OF FILE */
