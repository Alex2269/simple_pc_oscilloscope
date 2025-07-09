// sliders_ex.c

#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "psf_font.h"  // Підключення PSF шрифтів
extern PSF_Font font18;  // Зовнішній шрифт для тексту
extern int spacing;      // Відступ між символами у тексті

#define SLIDER_KNOB_SIZE 12    // Розмір ручки слайдера (ширина або висота залежно від орієнтації)
#define MAX_SLIDERS 10         // Максимальна кількість слайдерів, які одночасно можуть існувати

// Масив для збереження стану активності кожного слайдера за індексом sliderId
static bool slidersActiveStates[MAX_SLIDERS] = {0};

// Індекс слайдера, який зараз перетягується (-1, якщо перетягування відсутнє)
static int activeDraggingSlider = -1;

// Обчислення яскравості кольору (luminance) для вибору контрастного тексту
static float GetLuminance(Color color) {
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Вибір контрастного кольору тексту (чорний або білий) залежно від фону
static Color GetContrastingTextColor(Color bgColor) {
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}
/*
// Обчислення кількості символів у UTF-8 рядку
static int utf8_strlen(const char* s) {
    int len = 0;
    while (*s) {
        if ((*s & 0xc0) != 0x80) len++;
        s++;
    }
    return len;
}*/

// Головна функція слайдера з унікальним ідентифікатором sliderId
// sliderId - унікальний індекс слайдера (0..MAX_SLIDERS-1)
// bounds - прямокутник слайдера (позиція і розмір)
// font - шрифт для тексту
// textTop - текст підказки над слайдером (може бути NULL)
// textRight - текст праворуч від слайдера (може бути NULL)
// value - вказівник на поточне значення слайдера
// minValue, maxValue - межі значення слайдера
// isVertical - орієнтація слайдера (true - вертикальний, false - горизонтальний)
// baseColor - базовий колір слайдера
float Gui_SliderEx(int sliderId, Rectangle bounds, PSF_Font font, const char *textTop, const char *textRight,
                   float *value, float minValue, float maxValue, bool isVertical, Color baseColor) {
    // Перевірка коректності sliderId
    if (sliderId < 0 || sliderId >= MAX_SLIDERS) return *value;

    // Вказівник на стан активності слайдера за індексом
    bool *isActive = &slidersActiveStates[sliderId];

    // Отримуємо позицію миші
    Vector2 mousePos = GetMousePosition();

    // Обчислюємо нормалізоване значення слайдера (0..1)
    float normValue = (*value - minValue) / (maxValue - minValue);
    if (normValue < 0) normValue = 0;
    if (normValue > 1) normValue = 1;

    // Обчислення прямокутника ручки слайдера (knob)
    Rectangle knobRect;
    if (isVertical) {
        // Для вертикального слайдера ручка рухається по осі Y
        float knobY = bounds.y + (1.0f - normValue) * bounds.height;
        knobRect = (Rectangle){ bounds.x, knobY - SLIDER_KNOB_SIZE/2, bounds.width, SLIDER_KNOB_SIZE };
    } else {
        // Для горизонтального слайдера ручка рухається по осі X
        float knobX = bounds.x + normValue * bounds.width;
        knobRect = (Rectangle){ knobX - SLIDER_KNOB_SIZE/2, bounds.y, SLIDER_KNOB_SIZE, bounds.height };
    }

    // Створюємо розширену область прилипання (sticky area) ±10 пікселів по напрямку руху ручки
    Rectangle stickyRect = knobRect;
    const int stickyMargin = 10;  // Розмір області прилипання

    if (isVertical) {
        // Розширюємо область по вертикалі: вгору і вниз на 10 пікселів
        stickyRect.y -= stickyMargin;
        stickyRect.height += 2 * stickyMargin;
    } else {
        // Розширюємо область по горизонталі: вліво і вправо на 10 пікселів
        stickyRect.x -= stickyMargin;
        stickyRect.width += 2 * stickyMargin;
    }

    // Перевірка, чи знаходиться курсор миші в області прилипання ручки
    bool mouseOverSticky = CheckCollisionPointRec(mousePos, stickyRect);

    // Якщо натиснута ліва кнопка миші, курсор у зоні прилипання, і не перетягується інший слайдер
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOverSticky && activeDraggingSlider == -1) {
        *isActive = true;  // Активуємо перетягування для цього слайдера
        activeDraggingSlider = sliderId;  // Запам'ятовуємо індекс активного слайдера
    }

    // Якщо відпущена ліва кнопка миші і слайдер активний — завершуємо перетягування
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && *isActive) {
        *isActive = false;
        activeDraggingSlider = -1;
    }

    // Якщо слайдер активний і це саме він перетягується — оновлюємо значення слайдера
    if (*isActive && activeDraggingSlider == sliderId) {
        if (isVertical) {
            // Для вертикального слайдера інвертуємо координату Y, щоб верх відповідав maxValue
            normValue = 1.0f - (mousePos.y - bounds.y) / bounds.height;
        } else {
            // Для горизонтального слайдера беремо координату X
            normValue = (mousePos.x - bounds.x) / bounds.width;
        }
        // Обмежуємо нормалізоване значення в межах [0,1]
        if (normValue < 0) normValue = 0;
        if (normValue > 1) normValue = 1;

        // Обчислюємо реальне значення слайдера з урахуванням меж
        *value = minValue + normValue * (maxValue - minValue);
    }

    // Визначаємо колір фону слайдера з урахуванням активності і наведення миші
    Color sliderBgColor = baseColor;
    if (!(*isActive)) sliderBgColor = Fade(sliderBgColor, 0.4f);  // Зменшуємо яскравість, якщо не активний
    if (CheckCollisionPointRec(mousePos, bounds)) sliderBgColor = Fade(sliderBgColor, 0.8f);  // Трохи яскравіший при наведенні

    // Малюємо ручку слайдера (knob)
    Color knobColor = *isActive ? Fade(GetContrastingTextColor(baseColor), 0.5f) : baseColor;
    if (isVertical) {
        float knobY = bounds.y + (1.0f - normValue) * bounds.height;
        DrawRectangle(bounds.x, knobY - SLIDER_KNOB_SIZE/2, bounds.width, SLIDER_KNOB_SIZE, knobColor);
    } else {
        float knobX = bounds.x + normValue * bounds.width;
        DrawRectangle(knobX - SLIDER_KNOB_SIZE/2, bounds.y, SLIDER_KNOB_SIZE, bounds.height, knobColor);
    }

    // Повертаємо оновлене значення слайдера
    return *value;
}
