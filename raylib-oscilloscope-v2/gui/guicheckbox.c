// guicheckbox.c

#include "guicheckbox.h"
#include "raylib.h"

extern int fontSize;
extern int LineSpacing;
extern Font font;

// Функція для обчислення яскравості кольору
static float GetLuminance(Color color)
{
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

// Функція для вибору контрастного кольору (чорний або білий)
static Color GetContrastingTextColor(Color bgColor)
{
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}

// Проста функція чекбокса
// bounds - прямокутник чекбокса (для квадрата)
// checked - вказівник на стан чекбокса
// text - текст поруч із чекбоксом
// font - шрифт
// fontSize - розмір шрифту
void Gui_CheckBox(Rectangle bounds, bool *checked, const char *text, Color сolor)
{
    Vector2 mousePoint = GetMousePosition();
    bool mouseOver = CheckCollisionPointRec(mousePoint, bounds);

    Color boxColor = (*checked) ? сolor : LIGHTGRAY;
    if (mouseOver) boxColor = Fade(boxColor, 0.8f);

    DrawRectangleRec(bounds, boxColor);

    Color borderColor = GetContrastingTextColor(boxColor);
    DrawRectangleLinesEx((Rectangle){bounds.x - 2, bounds.y - 2, bounds.width + 4, bounds.height + 4}, 2, borderColor);

    if (*checked)
    {
        Vector2 p1 = {bounds.x + bounds.width*0.2f, bounds.y + bounds.height*0.5f};
        Vector2 p2 = {bounds.x + bounds.width*0.45f, bounds.y + bounds.height*0.75f};
        Vector2 p3 = {bounds.x + bounds.width*0.8f, bounds.y + bounds.height*0.25f};
        DrawLineEx(p1, p2, 3, borderColor);
        DrawLineEx(p2, p3, 3, borderColor);
    }

    Color textColor = GetContrastingTextColor(boxColor);

    Vector2 textSize = MeasureTextEx(font, text, fontSize, 1);
    Vector2 textPos = {bounds.x + bounds.width + 10, bounds.y + (bounds.height - textSize.y) / 2};

    // Малюємо фон для тексту з невеликим відступом (padding)
    int padding = 4;
    Rectangle textBg = {
        textPos.x - padding,
        textPos.y,
        textSize.x + 2 * padding,
        textSize.y
    };
    DrawRectangleRec(textBg, boxColor);               // фон під текстом - таким самим кольором, як чекбокс
    DrawRectangleLinesEx(textBg, 1, borderColor);     // контур фону тексту

    DrawTextEx(font, text, textPos, fontSize, 1, textColor);

    if (mouseOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        *checked = !(*checked);
    }
}
