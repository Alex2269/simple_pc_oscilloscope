// arrow_button.h

#ifndef ARROW_BUTTON_H
#define ARROW_BUTTON_H

#include "raylib.h"
#include "psf_font.h"

typedef enum {
    ARROW_UP,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT
} ArrowDirection;

// Функція малювання і обробки кнопки зі стрілкою
// bounds - квадратна область кнопки
// font - шрифт для підказки
// direction - напрямок стрілки
// isVertical - орієнтація кнопки (true - вертикальна, false - горизонтальна)
// value - вказівник на змінну для інкременту/декременту
// step - крок зміни значення
// minValue, maxValue - межі значення
// textTop - підказка зверху (показується при наведенні)
// textRight - текст справа (показується завжди)
// baseColor - базовий колір кнопки
// Повертає true, якщо кнопка була натиснута у цьому кадрі
bool Gui_ArrowButton(Rectangle bounds, PSF_Font font, ArrowDirection direction, bool isVertical,
                     int *value, int step, int minValue, int maxValue,
                     const char *textTop, const char *textRight, Color baseColor);

#endif // ARROW_BUTTON_H
