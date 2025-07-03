// arrow_button.c
#include "arrow_button.h"
#include <stdio.h>
#include <string.h>

extern int spacing;    // Відступ між символами
extern int LineSpacing; // Відступ між рядками

// Вибір контрастного кольору (білий або чорний)
static Color GetContrastingTextColor(Color bgColor) {
    float r = bgColor.r / 255.0f;
    float g = bgColor.g / 255.0f;
    float b = bgColor.b / 255.0f;
    float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    return (luminance > 0.5f) ? BLACK : WHITE;
}

// Малюємо стрілку у квадраті bounds з напрямком direction і кольором color
static void DrawArrow(Rectangle bounds, ArrowDirection direction, Color color) {
    Vector2 center = { bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f };
    float size = bounds.width * 0.45f; // трохи більший розмір

    Vector2 points[3];

    // Зсув координат для уникнення "мертвих зон"
    points[0].x += 0.5f; points[0].y += 0.5f;
    points[1].x += 0.5f; points[1].y += 0.5f;
    points[2].x += 0.5f; points[2].y += 0.5f;

    switch(direction) {
        case ARROW_UP:
            points[0] = (Vector2){ center.x, center.y - size };
            points[1] = (Vector2){ center.x - size, center.y + size };
            points[2] = (Vector2){ center.x + size, center.y + size };
            break;
        case ARROW_DOWN:
            points[0] = (Vector2){ center.x, center.y + size };
            points[1] = (Vector2){ center.x + size, center.y - size };
            points[2] = (Vector2){ center.x - size, center.y - size };
            break;
        case ARROW_LEFT:
            points[0] = (Vector2){ center.x - size, center.y };
            points[1] = (Vector2){ center.x + size, center.y + size };
            points[2] = (Vector2){ center.x + size, center.y - size };
            break;
        case ARROW_RIGHT:
            points[0] = (Vector2){ center.x + size, center.y };
            points[1] = (Vector2){ center.x - size, center.y - size };
            points[2] = (Vector2){ center.x - size, center.y + size };
            break;
    }

    DrawTriangle(points[0], points[1], points[2], color);
}

// Функція підрахунку довжини UTF-8 рядка (кількість символів)
// static int utf8_strlen(const char* s) {
//     int len = 0;
//     while (*s) {
//         if ((*s & 0xc0) != 0x80) len++;
//         s++;
//     }
//     return len;
// }

bool Gui_ArrowButton(Rectangle bounds, PSF_Font font, ArrowDirection direction, bool isVertical,
                     int *value, int step, int minValue, int maxValue,
                     const char *textTop, const char *textRight, Color baseColor) {
    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointRec(mousePos, bounds);
    bool pressed = false;

    Color btnColor = baseColor;
    if (mouseOver) btnColor = Fade(baseColor, 0.8f);
    if (mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) btnColor = Fade(baseColor, 0.6f);

    Color borderColor = GetContrastingTextColor(btnColor);
    int borderThickness = 2;

    // Малюємо подвійний контур для кращої видимості
    DrawRectangleLinesEx((Rectangle){bounds.x - borderThickness, bounds.y - borderThickness,
                                     bounds.width + 2*borderThickness, bounds.height + 2*borderThickness},
                         borderThickness, borderColor);
    DrawRectangleLinesEx((Rectangle){bounds.x - 2*borderThickness, bounds.y - 2*borderThickness,
                                     bounds.width + 4*borderThickness, bounds.height + 4*borderThickness},
                         borderThickness, (borderColor.r == 0 && borderColor.g == 0 && borderColor.b == 0) ? WHITE : BLACK);

    DrawRectangleRec(bounds, btnColor);

    // Колір стрілки — контрастний до фону кнопки
    Color arrowColor = GetContrastingTextColor(btnColor);
    // Якщо фон синій (приклад перевірки), зробити стрілку білою
    if (btnColor.b > 150 && btnColor.r < 100 && btnColor.g < 100) {
        arrowColor = WHITE;
    }


    DrawArrow(bounds, direction, arrowColor);

    // Малюємо підказку зверху при наведенні
    if (mouseOver && textTop && textTop[0] != '\0') {
        int padding = 6;
        int charCount = utf8_strlen(textTop);
        float textWidth = charCount * (font.width + spacing) - spacing;
        float boxWidth = textWidth + 2 * padding;
        float boxHeight = font.height + 2 * padding;

        Rectangle tooltipRect = {
            bounds.x + bounds.width / 2.0f - boxWidth / 2.0f,
            bounds.y - boxHeight - 8,
            boxWidth,
            boxHeight
        };

        DrawRectangleRec(tooltipRect, Fade(borderColor, 0.9f));
        DrawRectangleLinesEx(tooltipRect, 1, GetContrastingTextColor(borderColor));
        DrawPSFText(font, tooltipRect.x + padding, tooltipRect.y + padding / 2, textTop, spacing, GetContrastingTextColor(borderColor));
    }

    // Малюємо текст праворуч (для вертикального розміщення) або під кнопкою (горизонтального)
    if (textRight && textRight[0] != '\0') {
        int padding = 6;
        int charCount = utf8_strlen(textRight);
        float textWidth = charCount * (font.width + spacing) - spacing;
        float boxWidth = textWidth + 2 * padding;
        float boxHeight = font.height + 2 * padding;

        Rectangle textRect;
        if (isVertical) {
            // Праворуч, по центру по вертикалі
            textRect.x = bounds.x + bounds.width + 12;
            textRect.y = bounds.y + bounds.height / 2.0f - boxHeight / 2.0f;
            textRect.width = boxWidth;
            textRect.height = boxHeight;
        } else {
            // Під кнопкою, по центру по горизонталі
            textRect.x = bounds.x + bounds.width / 2.0f - boxWidth / 2.0f;
            textRect.y = bounds.y + bounds.height + 8;
            textRect.width = boxWidth;
            textRect.height = boxHeight;
        }

        DrawRectangleRec(textRect, Fade(borderColor, 0.9f));
        DrawRectangleLinesEx(textRect, 1, GetContrastingTextColor(borderColor));
        DrawPSFText(font, textRect.x + padding, textRect.y + padding / 2, textRight, spacing, GetContrastingTextColor(borderColor));
    }

    // Обробка натискання: при натисканні інкрементуємо або декрементуємо значення
    if (mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (direction == ARROW_UP || direction == ARROW_RIGHT) {
            *value += step;
            if (*value > maxValue) *value = maxValue;
        } else {
            *value -= step;
            if (*value < minValue) *value = minValue;
        }
        pressed = true;
    }

    return pressed;
}
