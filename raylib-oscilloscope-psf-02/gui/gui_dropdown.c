// gui_dropdown.c
#include "gui_dropdown.h"
#include "button.h"        // Залежить від Gui_Button, тож потрібен.
#include "raylib.h"
#include <string.h>

extern int fontSize;       // Зовнішня змінна розміру шрифту
extern int LineSpacing;    // Відступ між рядками тексту
extern Font font;          // Зовнішній шрифт для малювання тексту

#include "psf_font.h"      // Заголовок із парсером PSF-шрифту
// #include "GlyphCache.h"
extern PSF_Font font32;    // Глобальна змінна шрифту PSF
extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

/**
 * @brief Обчислює яскравість кольору (luminance) за формулою зваженого середнього.
 *
 * @param color Колір у форматі raylib Color
 * @return float Значення яскравості від 0 (темний) до 1 (світлий)
 */
static float GetLuminance(Color color)
{
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

/**
 * @brief Вибирає контрастний колір тексту (чорний або білий) залежно від яскравості фону.
 *
 * @param bgColor Колір фону
 * @return Color Чорний або білий колір для кращої читабельності
 */
static Color GetContrastingTextColor(Color bgColor)
{
    return (GetLuminance(bgColor) > 0.5f) ? BLACK : WHITE;
}

int Gui_Dropdown(Rectangle bounds, const char **items, int itemCount,
                 int currentIndex, const char *hintText, bool *isOpen, Color color) {
    int selectedIndex = currentIndex;

    // --- Малюємо підказку зверху (hintText) при наведенні миші ---
    Vector2 mousePoint = GetMousePosition();               // Поточна позиція миші
    bool mouseOver = CheckCollisionPointRec(mousePoint, bounds);  // Чи наведена миша на чекбокс

    Color textColor = GetContrastingTextColor(color);  // Колір тексту для читабельності
    int localSpacing = 2;  // Відступ між символами (локальна змінна для уникнення конфліктів)
    int padding = 4;       // Відступи навколо тексту (padding)

    if (mouseOver && hintText && hintText[0] != '\0')
    {
        // Розбиття тексту підказки на рядки (максимум 10 рядків)
        const char* lines[10];
        int lineCountTop = 0;

        char tempText[256];  // Тимчасовий буфер для копії тексту підказки (щоб не змінювати оригінал)
        strncpy(tempText, hintText, sizeof(tempText) - 1);
        tempText[sizeof(tempText) - 1] = '\0';

        char* line = strtok(tempText, "\n");
        while (line != NULL && lineCountTop < 10) {
            lines[lineCountTop++] = line;
            line = strtok(NULL, "\n");
        }

        // Обчислення максимальної ширини рядка з урахуванням кількості символів UTF-8
        float maxWidthTop = 0;
        for (int i = 0; i < lineCountTop; i++) {
            int charCount = utf8_strlen(lines[i]);  // Кількість символів у рядку UTF-8
            float lineWidth = charCount * (font32.width + localSpacing) - localSpacing;
            if (lineWidth > maxWidthTop) maxWidthTop = lineWidth;
        }

        float lineHeightTop = (float)font32.height;
        // Загальна висота підказки з урахуванням міжрядкових відступів і padding
        float totalHeightTop = lineCountTop * lineHeightTop + (lineCountTop - 1) * 2 + 2 * padding;

        // Прямокутник фону підказки зверху, центрований по горизонталі над чекбоксом
        Rectangle tooltipRect = {
            bounds.x + bounds.width / 2.0f - (maxWidthTop + 2 * padding) / 2.0f,
            bounds.y - totalHeightTop - 8,  // Відступ зверху 8 пікселів
            maxWidthTop + 2 * padding,
            totalHeightTop
        };

        // Малюємо фон підказки з напівпрозорим чорним кольором
        DrawRectangleRec(tooltipRect, Fade(BLACK, 0.8f));
        // Малюємо рамку підказки білим кольором
        DrawRectangleLinesEx(tooltipRect, 1, WHITE);

        // Малюємо кожен рядок тексту підказки з вертикальним інтервалом 2 пікселі
        for (int i = 0; i < lineCountTop; i++) {
            DrawPSFText(font32,
                            tooltipRect.x + padding,
                            tooltipRect.y + padding / 2 + i * (lineHeightTop + 2),
                            lines[i], localSpacing, WHITE);
        }
    }

    // Малюємо кнопку dropdown з поточним вибором
    if (Gui_Button(bounds, items[selectedIndex], color, GRAY, DARKGRAY, (Color){0,0,0,0})) {
        *isOpen = !(*isOpen);
    }

    if (*isOpen) {
        // Малюємо список пунктів під кнопкою
        for (int i = 0; i < itemCount; i++) {
            Rectangle itemRect = {
                bounds.x,
                bounds.y + bounds.height * (i + 1),
                bounds.width,
                bounds.height
            };

            Color btnColor = (i == selectedIndex) ? color : Fade(color, 0.5f);

            if (Gui_Button(itemRect, items[i], btnColor, GRAY, DARKGRAY, (Color){0,0,0,0})) {
                printf("Selected index: %d, item: %s\n", i, items[i]);
                selectedIndex = i;
                *isOpen = false;
            }
        }
    }

    return selectedIndex;
}

