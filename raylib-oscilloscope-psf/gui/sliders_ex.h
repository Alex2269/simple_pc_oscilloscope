// sliders_ex.h

#ifndef SLIDERS_EX_H
#define SLIDERS_EX_H

#include "raylib.h"
#include "psf_font.h"      // Заголовок із парсером PSF-шрифту

// Функція, що малює слайдер, обробляє введення і повертає оновлене значення
// minValue, maxValue — діапазон значень слайдера
// value — поточне значення (передається і повертається)
// isVertical — орієнтація слайдера
// sliderId - індекс слайдера для унікальної ідентифікації
float Gui_SliderEx(int sliderId, Rectangle bounds, PSF_Font font, const char *textTop, const char *textRight,
                   float *value, float minValue, float maxValue, bool isVertical, Color colorText);

#endif // SLIDERS_EX_H
