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

typedef struct {
    bool isHeld;
    double holdStartTime;
    double lastUpdateTime;
    double accumulatedTime;
} HoldState;

// Функція малювання і обробки кнопки зі стрілкою з підтримкою утримання
// Параметри:
// - bounds: область кнопки
// - font: PSF-шрифт для тексту
// - direction: напрямок стрілки
// - isVertical: орієнтація кнопки (true - вертикальна, false - горизонтальна)
// - value: вказівник на змінну для зміни
// - step: крок зміни значення (зазвичай 1)
// - minValue, maxValue: межі значення
// - textTop: підказка зверху (при наведенні)
// - textRight: текст праворуч (завжди)
// - baseColor: колір кнопки
// - holdState: стан утримання (повинен бути унікальним для кожної кнопки)
// Повертає true, якщо значення змінилося у цьому кадрі
bool Gui_ArrowButton(Rectangle bounds, PSF_Font font, ArrowDirection direction, bool isVertical,
                     int *value, int step, int minValue, int maxValue,
                     const char *textTop, const char *textRight, Color baseColor,
                     HoldState *holdState);

#endif // ARROW_BUTTON_H

