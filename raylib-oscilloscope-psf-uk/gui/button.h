// button.h
#ifndef BUTTON_H
#define BUTTON_H

#include "raylib.h"

// Зовнішні змінні шрифту
extern Font font;
extern int fontSize;
extern int LineSpacing;

// Функція кнопки з автоматичним підбором кольору тексту
bool Gui_Button(Rectangle bounds, const char *text,
                Color colorNormal, Color colorHover, Color colorPressed, Color colorText);

#endif // BUTTON_H
