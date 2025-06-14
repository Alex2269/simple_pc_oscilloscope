// knob_gui.c

#include "knob_gui.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979323846f
#define CHANNEL_COUNT 6

extern int fontSize;  // Зовнішня змінна розміру шрифту
extern Font font;     // Зовнішній шрифт для малювання тексту

// Обчислення яскравості кольору (luminance)
static float GetLuminance(Color color);

// Вибір контрастного кольору тексту (білий або чорний)
static Color GetContrastingTextColor(Color bgColor);

// Масив кутів повороту регуляторів для кожного каналу (початково -135 градусів)
static float knobAngles[CHANNEL_COUNT] = { -135.0f, -135.0f, -135.0f, -135.0f, -135.0f, -135.0f };

// Масив станів перетягування (dragging) для кожного каналу
static bool isDragging[CHANNEL_COUNT] = { false, false, false, false, false, false };

// Індекс активного каналу, який зараз перетягують (-1 означає, що ніхто не перетягує)
static int activeDraggingChannel = -1;

/**
 * @brief Малює поворотний регулятор із індикатором кута та шкалою.
 *
 * @param x_pos Координата X центру регулятора
 * @param y_pos Координата Y центру регулятора
 * @param radius Радіус регулятора
 * @param knobAngle Поточний кут повороту регулятора в градусах (-135..135)
 * @param knobValue Поточне значення регулятора (float у межах minValue..maxValue)
 * @param colorText Колір тексту для відображення значення регулятора
 */
void draw_knob(uint16_t x_pos, uint16_t y_pos, float radius, float knobAngle, float knobValue, Color colorText) {
    Vector2 center = { (float)x_pos, (float)y_pos };

    // Малюємо зовнішнє коло світло-сірим кольором
    DrawCircleV(center, radius, LIGHTGRAY);
    // Малюємо внутрішнє коло темно-сірим кольором для об'єму
    DrawCircleV(center, radius - 5, DARKGRAY);

    // Обчислюємо позицію індикатора кута повороту регулятора
    float rad = (knobAngle - 90.0f) * (PI / 180.0f);
    Vector2 indicator = { center.x + cosf(rad) * (radius - 10), center.y + sinf(rad) * (radius - 10) };
    // Малюємо індикатор червоною лінією
    DrawLineEx(center, indicator, 4, RED);

    // Малюємо шкалу з рисками через кожні 5 одиниць (крок 5%)
    for (int i = 0; i <= 100; i += 5) {
        float tickAngle = -135.0f + (i / 100.0f) * 270.0f;
        float tickRad = (tickAngle - 90.0f) * (PI / 180.0f);
        float innerRadius = (i % 10 == 0) ? radius - 8 : radius - 4;
        float outerRadius = radius;
        Vector2 start = { center.x + cosf(tickRad) * innerRadius, center.y + sinf(tickRad) * innerRadius };
        Vector2 end = { center.x + cosf(tickRad) * outerRadius, center.y + sinf(tickRad) * outerRadius };
        DrawLineEx(start, end, 2, BLACK);
    }

    // Відображення значення регулятора з одним знаком після коми
    // DrawTextEx(font, TextFormat("%.1f", knobValue),
    //            (Vector2){center.x + radius + 10, center.y - radius / 2},
    //            fontSize, 2, colorText);
}

/**
 * @brief Обробляє взаємодію користувача з регулятором: натискання, перетягування, відпускання миші.
 *
 * @param x_pos Координата X центру регулятора
 * @param y_pos Координата Y центру регулятора
 * @param radius Радіус регулятора
 * @param knobValue Поточне значення регулятора (float)
 * @param isDragging Вказівник на стан перетягування (true/false)
 * @param knobAngle Вказівник на кут повороту регулятора (float)
 * @param isActive Чи активний регулятор (чи реагує на події)
 * @param channel Номер каналу (0..3)
 * @param minValue Мінімальне значення регулятора (float)
 * @param maxValue Максимальне значення регулятора (float)
 * @return float Оновлене значення регулятора
 */
float knob_handler(uint16_t x_pos, uint16_t y_pos, float radius, float knobValue, bool *isDragging,
                   float *knobAngle, bool isActive, int channel, float minValue, float maxValue) {
    Vector2 center = { (float)x_pos, (float)y_pos };
    Vector2 mousePos = GetMousePosition();

    // Якщо регулятор неактивний, повертаємо поточне значення без змін
    if (!isActive) return knobValue;

    // Перевіряємо, чи курсор миші знаходиться над регулятором
    bool mouseOver = CheckCollisionPointCircle(mousePos, center, radius);

    // Обчислюємо кут між центром регулятора і курсором миші в градусах
    float angle = atan2f(mousePos.y - center.y, mousePos.x - center.x) * (180.0f / PI);
    angle -= 90.0f + 180.0f;

    // Коригуємо кут для діапазону від -135 до 135 градусів
    if (angle < -180.0f) angle += 360.0f;
    if (angle < -135.0f) angle = -135.0f;
    if (angle > 135.0f) angle = 135.0f;

    // Починаємо перетягування, якщо натиснута ліва кнопка миші, курсор над регулятором і ніхто не перетягує
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouseOver && activeDraggingChannel == -1) {
        *isDragging = true;
        activeDraggingChannel = channel; // Запам'ятовуємо активний канал
    }

    // Завершуємо перетягування при відпусканні кнопки миші
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && *isDragging) {
        *isDragging = false;
        activeDraggingChannel = -1; // Немає активного перетягування
    }

    // Якщо триває перетягування, оновлюємо кут і значення регулятора
    if (*isDragging) {
        *knobAngle = angle;

        // Обчислюємо нормалізоване значення 0..1 на основі кута
        float normalizedValue = (*knobAngle + 135.0f) / 270.0f;

        // Масштабуємо нормалізоване значення у діапазон minValue..maxValue
        knobValue = minValue + normalizedValue * (maxValue - minValue);

        // Обмеження значення в межах minValue..maxValue
        if (knobValue < minValue) knobValue = minValue;
        if (knobValue > maxValue) knobValue = maxValue;
    }

    return knobValue;
}

/**
 * @brief Головна функція для відображення і обробки поворотного регулятора для конкретного каналу з діапазоном значень і кольором тексту.
 *
 * @param channel Номер каналу (0..3)
 * @param x_pos Координата X центру регулятора
 * @param y_pos Координата Y центру регулятора
 * @param textTop Текст підказки зверху
 * @param textRight Текст, що відображається зправа
 * @param radius Радіус регулятора
 * @param value Вказівник на значення регулятора (float)
 * @param minValue Мінімальне значення регулятора (float)
 * @param maxValue Максимальне значення регулятора (float)
 * @param isActive Чи активний регулятор (bool)
 * @param colorText Колір тексту значення регулятора (Color)
 * @return int 1, якщо регулятор відображено і оброблено, 0 - якщо канал недійсний
 */
int Gui_Knob_Channel(int channel, int x_pos, int y_pos, const char *textTop, const char *textRight,
                     float radius, float *value, float minValue, float maxValue, bool isActive, Color colorText)
{
    // Перевірка коректності номера каналу
    if (channel < 0 || channel >= CHANNEL_COUNT) return 0;

    // Викликаємо обробник з новими параметрами minValue і maxValue
    float changed = knob_handler(x_pos, y_pos, radius, *value, &isDragging[channel], &knobAngles[channel], isActive, channel, minValue, maxValue);

    // Записуємо оновлене значення назад
    bool valueChanged = (changed != *value);
    *value = changed;

    // Оновлення кута, якщо регулятор не перетягується (щоб відобразити зовнішні зміни значення)
    if (!isDragging[channel]) {
        knobAngles[channel] = ((*value - minValue) / (maxValue - minValue)) * 270.0f - 135.0f;
    }

    draw_knob(x_pos, y_pos, radius, knobAngles[channel], *value, colorText);

    Vector2 center = { (float)x_pos, (float)y_pos };
    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointCircle(mousePos, center, radius);

    // Визначаємо колір тексту (автоматично підбираємо, якщо альфа 0)
    Color textColor = GetContrastingTextColor(colorText);

    // Відображення підказки зверху при наведенні курсора миші
    if (mouseOver && textTop && strlen(textTop) > 0) {
        int padding = 6; // Відступи навколо тексту підказки
        Vector2 textSize = MeasureTextEx(font, textTop, fontSize, 2);

        // Прямокутник для фону підказки textTop
        Rectangle tooltipRect = {
            center.x - (textSize.x / 2.0f) - padding,
            center.y - radius - textSize.y - 2 * padding - 10,
            textSize.x + 2 * padding,
            textSize.y + 2 * padding
        };

        // Малюємо фоновий прямокутник підказки textTop
        DrawRectangleRec(tooltipRect, textColor);
        // Малюємо рамку підказки
        DrawRectangleLinesEx(tooltipRect, 1, DARKGRAY);
        // Малюємо текст підказки білим кольором
        DrawTextEx(font, textTop,
                   (Vector2){tooltipRect.x + padding, tooltipRect.y},
                   fontSize, 2, colorText);
    }

    int padding = 6; // Відступи навколо тексту підказки
    Vector2 textSize = MeasureTextEx(font, textRight, fontSize, 2);

    // Прямокутник для фону підказки textRight
    Rectangle tooltipRect = {
        center.x + radius + 4,
        center.y - radius / 2 - 4,
        textSize.x + 2 * padding,
        textSize.y + 2 * padding
    };

    // Малюємо фоновий прямокутник підказки textRight
    DrawRectangleRec(tooltipRect, colorText);
    // Малюємо рамку підказки
    DrawRectangleLinesEx(tooltipRect, 1, DARKGRAY);
    // Малюємо текст праворуч регулятора
    DrawTextEx(font, textRight,
               (Vector2){center.x + radius + 10, center.y - radius / 2},
               fontSize, 2, textColor);

    return valueChanged ? 1 : 0;
}

// Обчислення яскравості кольору (luminance)
static float GetLuminance(Color color)
{
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Вибір контрастного кольору тексту (білий або чорний)
static Color GetContrastingTextColor(Color bgColor)
{
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}

