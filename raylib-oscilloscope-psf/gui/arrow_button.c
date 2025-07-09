// arrow_button.с

#define _POSIX_C_SOURCE 199309L
#include <time.h>

#include "arrow_button.h"
#include <stdio.h>

// Зовнішні змінні для відступів у PSF-шрифті (визначені у вашому коді)
extern int spacing;    // Відступ між символами
extern int LineSpacing; // Відступ між рядками

static double GetSystemTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static Color GetContrastingTextColor(Color bgColor) {
    float r = bgColor.r / 255.0f;
    float g = bgColor.g / 255.0f;
    float b = bgColor.b / 255.0f;
    float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    return (luminance > 0.5f) ? BLACK : WHITE;
}

static void DrawArrow(Rectangle bounds, ArrowDirection direction, Color color) {
    Vector2 center = { bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f };
    float size = bounds.width * 0.45f;

    Vector2 points[3];

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
/*
// Функція підрахунку довжини UTF-8 рядка (кількість символів)
static int utf8_strlen(const char* s) {
    int len = 0;
    while (*s) {
        if ((*s & 0xc0) != 0x80) len++;
        s++;
    }
    return len;
}*/

bool Gui_ArrowButton(Rectangle bounds, PSF_Font font, ArrowDirection direction, bool isVertical,
                     int *value, int step, int minValue, int maxValue,
                     const char *textTop, const char *textRight, Color baseColor,
                     HoldState *holdState) {
    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointRec(mousePos, bounds);

    Color btnColor = baseColor;
    if (mouseOver) btnColor = Fade(baseColor, 0.8f);
    if (mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) btnColor = Fade(baseColor, 0.6f);

    Color borderColor = GetContrastingTextColor(btnColor);
    int borderThickness = 2;

    DrawRectangleLinesEx((Rectangle){bounds.x - borderThickness, bounds.y - borderThickness,
                                     bounds.width + 2*borderThickness, bounds.height + 2*borderThickness},
                         borderThickness, borderColor);
    DrawRectangleLinesEx((Rectangle){bounds.x - 2*borderThickness, bounds.y - 2*borderThickness,
                                     bounds.width + 4*borderThickness, bounds.height + 4*borderThickness},
                         borderThickness, (borderColor.r == 0 && borderColor.g == 0 && borderColor.b == 0) ? WHITE : BLACK);

    DrawRectangleRec(bounds, btnColor);

    Color arrowColor = GetContrastingTextColor(btnColor);
    if (btnColor.b > 150 && btnColor.r < 100 && btnColor.g < 100) {
        arrowColor = WHITE;
    }

    DrawArrow(bounds, direction, arrowColor);

    // Підказка зверху при наведенні
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

    // Текст праворуч (для вертикальної) або під кнопкою (горизонтальної)
    if (textRight && textRight[0] != '\0') {
        int padding = 6;
        int charCount = utf8_strlen(textRight);
        float textWidth = charCount * (font.width + spacing) - spacing;
        float boxWidth = textWidth + 2 * padding;
        float boxHeight = font.height + 2 * padding;

        Rectangle textRect;
        if (isVertical) {
            textRect.x = bounds.x + bounds.width + 12;
            textRect.y = bounds.y + bounds.height / 2.0f - boxHeight / 2.0f;
            textRect.width = boxWidth;
            textRect.height = boxHeight;
        } else {
            textRect.x = bounds.x + bounds.width / 2.0f - boxWidth / 2.0f;
            textRect.y = bounds.y + bounds.height + 8;
            textRect.width = boxWidth;
            textRect.height = boxHeight;
        }

        DrawRectangleRec(textRect, Fade(borderColor, 0.9f));
        DrawRectangleLinesEx(textRect, 1, GetContrastingTextColor(borderColor));
        DrawPSFText(font, textRect.x + padding, textRect.y + padding / 2, textRight, spacing, GetContrastingTextColor(borderColor));
    }

    // --- Логіка натискання з утриманням і прискоренням ---

    double currentTime = GetSystemTime();

    if (!holdState->isHeld) {
        holdState->lastUpdateTime = currentTime;
        holdState->accumulatedTime = 0.0;
    }

    double deltaTime = currentTime - holdState->lastUpdateTime;
    holdState->lastUpdateTime = currentTime;

    bool pressed = false;

    bool mousePressed = mouseOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool mouseDown = mouseOver && IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    static const double delayBeforeAccel = 0.175;  // Затримка перед прискоренням (сек)
    static const double baseInterval = 0.25;       // Початковий інтервал (сек)
    static const double minInterval = 0.005;       // Мінімальний інтервал (сек)
    static const double accelRate = 0.075;         // Швидкість зменшення інтервалу

    if (mousePressed) {
        holdState->isHeld = true;
        holdState->holdStartTime = currentTime;
        holdState->accumulatedTime = 0.0;

        // Перший крок
        if ((direction == ARROW_UP || direction == ARROW_RIGHT) && *value < maxValue) {
            *value += step;
            pressed = true;
        } else if ((direction == ARROW_DOWN || direction == ARROW_LEFT) && *value > minValue) {
            *value -= step;
            pressed = true;
        }
    } else if (mouseDown && holdState->isHeld) {
        double holdDuration = currentTime - holdState->holdStartTime;

        double interval = baseInterval;
        if (holdDuration > delayBeforeAccel) {
            double accelTime = holdDuration - delayBeforeAccel;
            interval = baseInterval - accelTime * accelRate;
            if (interval < minInterval) interval = minInterval;
        }

        holdState->accumulatedTime += deltaTime;

        while (holdState->accumulatedTime >= interval) {
            holdState->accumulatedTime -= interval;

            if ((direction == ARROW_UP || direction == ARROW_RIGHT) && *value < maxValue) {
                *value += step;
                pressed = true;
            } else if ((direction == ARROW_DOWN || direction == ARROW_LEFT) && *value > minValue) {
                *value -= step;
                pressed = true;
            }
        }
    } else {
        holdState->isHeld = false;
        holdState->accumulatedTime = 0.0;
    }

    return pressed;
}

