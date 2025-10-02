/*
   Масиви для кнопок: Замінено десятки окремих прямокутників на масив buttons,
   який містить усі координати та розміри кнопок.

   Аналогічно створено масиви для міток кнопок buttonLabels та кольорів buttonColors.

   Функція для малювання кнопок: DrawButton тепер малює кнопку на основі переданих параметрів:
   прямокутник, стан кнопки, текст та колір.

   Функція для обробки натискання: HandleButtonClick обробляє натискання на будь-яку кнопку,
   змінюючи її стан.

   Використання циклів: У функціях DrawButtons Там рукоятка HandleButtonClicks
   використовуються цикли для обробки всіх 10 кнопок.

   Цей підхід дозволяє обробляти 10 кнопок за допомогою двох функцій замість 20.

 */

#include "button.h"
#include "raylib.h"

// Масив станів кнопок
bool buttonStates[10] = { false, false, false, false, false,
                          false, false, false, false, false };

// Массив прямокутників для кнопок
Rectangle buttons[10] = {
    {50, 50, 16, 16},
    {50, 90, 16, 16},
    {50, 130, 16, 16},
    {50, 170, 16, 16},
    {50, 210, 16, 16},
    {50, 250, 16, 16},
    {50, 290, 16, 16},
    {50, 330, 16, 16},
    {50, 370, 16, 16},
    {50, 410, 16, 16}
};

// массив надписів для кнопок
const char* buttonLabels[10] = {
    "Ch 1", "Ch 2", "Bt 3",
    "Bt 4", "Bt 5", "Bt 6",
    "Bt 7", "Bt 8", "Bt 9",
    "Bt 10"
};

Color buttonColors[10] = {
    YELLOW, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN
};

// Функція для малювання кнопок
void DrawButton(Rectangle buttonRect, bool buttonState, const char* label, Color color)
{
    Color buttonColor = buttonState ? color : LIGHTGRAY;
    DrawRectangleRec(buttonRect, buttonColor);
    DrawText(label, (int)(buttonRect.x + 25), (int)(buttonRect.y), 20, buttonState ? color : LIGHTGRAY);
}

// Ініціалізація кнопок вже здійснюється через масиви
void buttons_init(void)
{
    for (int i = 0; i < 10; i++)
    {
        Rectangle buttons[i];
    }
}

// Виклик малювання кнопок
void DrawButtons(void)
{
    for (int i = 0; i < 10; i++)
    {
        DrawButton(buttons[i], buttonStates[i], buttonLabels[i], buttonColors[i]);
    }
}

// Обробка натискання на кнопки
void HandleButtonClicks(void)
{
    for (int i = 0; i < 10; i++)
    {
        HandleButtonClick(buttons[i], &buttonStates[i]);
    }
}

// Функція для обробки натискання кнопок
void HandleButtonClick(Rectangle buttonRect, bool* buttonState)
{
    if (CheckCollisionPointRec(GetMousePosition(), buttonRect))
    {
        *buttonState = !*buttonState;
    }
}
