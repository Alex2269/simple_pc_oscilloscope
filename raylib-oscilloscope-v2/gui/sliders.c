// sliders.c
#include "sliders.h"
#include <string.h> // для memcmp

#define SLIDER_KNOB_SIZE 10
#define MAX_SLIDERS 10

typedef struct {
    Rectangle bounds;
    bool isActive;
    bool used;
} SliderState;

static SliderState slidersState[MAX_SLIDERS] = {0};

// Функція шукає або додає слайдер за bounds і повертає вказівник на його isActive
static bool* GetSliderActiveState(Rectangle bounds) {
    for (int i = 0; i < MAX_SLIDERS; i++) {
        if (slidersState[i].used) {
            // Порівнюємо bounds, щоб знайти існуючий слайдер
            if (memcmp(&slidersState[i].bounds, &bounds, sizeof(Rectangle)) == 0) {
                return &slidersState[i].isActive;
            }
        } else {
            // Вільний слот - додаємо новий слайдер
            slidersState[i].bounds = bounds;
            slidersState[i].isActive = false;
            slidersState[i].used = true;
            return &slidersState[i].isActive;
        }
    }
    return NULL; // Перевищено максимальну кількість слайдерів
}

extern int fontSize;
extern int LineSpacing;
extern Font font;

float Gui_Slider(Rectangle bounds, const char *textTop, const char *textRight,
                 float *value, float minValue, float maxValue, bool isVertical, Color colorText) {
    bool *isActive = GetSliderActiveState(bounds);
    if (isActive == NULL) return 0; // Не вдалося зареєструвати слайдер

    Vector2 mousePos = GetMousePosition();

    // Нормалізоване значення від 0 до 1
    float normValue = (*value - minValue) / (maxValue - minValue);

    bool mouseOver = CheckCollisionPointRec(mousePos, bounds);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOver) {
        *isActive = true;
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        *isActive = false;
    }

    if (*isActive) {
        if (isVertical) {
            normValue = (mousePos.y - bounds.y) / bounds.height;
        } else {
            normValue = (mousePos.x - bounds.x) / bounds.width;
        }
        if (normValue < 0) normValue = 0;
        if (normValue > 1) normValue = 1;

        *value = minValue + normValue * (maxValue - minValue);
    }

    DrawRectangleRoundedLines(bounds, 0.3f, 8, GRAY);

    Color knobColor = *isActive ? RED : SKYBLUE;

    if (isVertical) {
        float knobY = bounds.y + normValue * bounds.height;
        DrawRectangle(bounds.x, knobY - SLIDER_KNOB_SIZE / 2, bounds.width, SLIDER_KNOB_SIZE, knobColor);
    } else {
        float knobX = bounds.x + normValue * bounds.width;
        DrawRectangle(knobX - SLIDER_KNOB_SIZE / 2, bounds.y, SLIDER_KNOB_SIZE, bounds.height, knobColor);
    }

    // Малюємо текст зверху слайдера
    DrawTextEx(font, textTop,
               (Vector2){bounds.x, bounds.y - bounds.height},
               fontSize, LineSpacing, colorText);

    // Малюємо текст праворуч слайдера
    DrawTextEx(font, textRight,
               (Vector2){bounds.x + bounds.width + 10, bounds.y},
               fontSize, LineSpacing, colorText);

    // Тепер функція нічого не повертає, бо оновлює значення через вказівник
    return 0; // Можна змінити на void, якщо не потрібен return
}
