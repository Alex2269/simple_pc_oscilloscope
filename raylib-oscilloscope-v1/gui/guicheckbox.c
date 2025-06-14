// guicheckbox.c

#include "guicheckbox.h"
#include "raylib.h"

// Функції для контрастного кольору (копія з button.c)
static float GetLuminance(Color color)
{
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

static Color GetContrastingTextColor(Color bgColor)
{
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}

extern int fontSize;
extern int LineSpacing;
extern Font font;

void Gui_CheckBox(Rectangle bounds, bool *checked, const char *text, Color colorText)
{
    Vector2 mousePoint = GetMousePosition();

    Color boxColor = (*checked) ? GREEN : LIGHTGRAY;

    // Малюємо фон чекбокса
    DrawRectangleRec(bounds, boxColor);

    // Малюємо внутрішній контур товщиною 2 пікселі
    int borderThickness = 2;
    Color borderColor = GetContrastingTextColor(boxColor);

    for (int i = 0; i < borderThickness; i++)
    {
        // Використовуємо DrawRectangleLines (параметри: x, y, width, height)
        DrawRectangleLines(
            (int)(bounds.x + i),
            (int)(bounds.y + i),
            (int)(bounds.width - 2*i),
            (int)(bounds.height - 2*i),
            borderColor
        );
    }

    // Малюємо текст поруч із чекбоксом
    DrawTextEx(font, text,
               (Vector2){bounds.x + bounds.width + 10, bounds.y},
               fontSize, LineSpacing, colorText);

    // Обробляємо натискання миші
    if (CheckCollisionPointRec(mousePoint, bounds))
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            *checked = !(*checked);
        }
    }
}

