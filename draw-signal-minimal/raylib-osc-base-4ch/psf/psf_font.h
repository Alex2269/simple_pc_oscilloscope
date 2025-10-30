#ifndef PSF_FONT_H
#define PSF_FONT_H

#include "raylib.h"
#include <stdint.h>
#include <stddef.h>

// Структура шрифту PSF1/PSF2
typedef struct {
    int isPSF2;                 // Прапорець: 0 - шрифт формату PSF1, 1 - PSF2
    int glyph_width;            // Ширина символу в пікселях
    int glyph_height;           // Висота символу в пікселях
    int glyph_count;            // Кількість гліфів (символів) у шрифті
    int glyph_bytes;            // Розмір одного гліфа в байтах
    unsigned char* glyphBuffer; // Вказівник на буфер з бінарними даними гліфів
} PSF_Font;

// Функція завантаження PSF шрифту з файлу за шляхом filename
PSF_Font LoadPSFFont(const char* filename);

// Функція звільнення пам’яті, виділеної під шрифт
void UnloadPSFFont(PSF_Font font);

// Функція для відображення одного символу (гліфа) у позиції (x,y) заданим кольором
void DrawPSFChar(PSF_Font font, int x, int y, int c, Color color);

// Функція для відображення тексту у форматі UTF-8 шрифтом PSF з підтримкою кирилиці
void DrawPSFText(PSF_Font font, int x, int y, const char* text, int spacing, Color color);

void DrawPSFCharScaled(PSF_Font font, int x, int y, int c, int scale, Color color);
void DrawPSFTextScaled(PSF_Font font, int x, int y, const char* text, int spacing, int scale, Color color);

// Підрахунок кількості UTF-8 символів у рядку
int utf8_strlen(const char* s);

// Функція малювання одного символу (гліфа) у позиції (x,y) кольором color
void DrawPSFChar(PSF_Font font, int x, int y, int c, Color color);

void DrawPSFCharLine(PSF_Font font, int x, int y, const char* text, int spacing, Color color);

void DrawPSFTextWithInvertedBackground(PSF_Font font, int x, int y, const char* text,
                                       int spacing, Color textColor, int padding, int borderThickness);

void DrawPSFTextScaledWithInvertedBackground(PSF_Font font, int x, int y, const char* text,
                                             int spacing, int scale, Color textColor, int padding, int borderThickness);
#endif // PSF_FONT_H
