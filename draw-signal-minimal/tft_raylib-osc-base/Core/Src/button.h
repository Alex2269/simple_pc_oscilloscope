
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

#ifndef BUTTON_H
#define BUTTON_H

#include "button.h"
#include "raylib.h"


// Масив станів кнопок
extern bool buttonStates[10];
// Массив прямокутників для кнопок
extern Rectangle buttons[10];
// массив надписів для кнопок
extern const char* buttonLabels[10];
extern Color buttonColors[10];

// Функція для малювання кнопок
void DrawButton(Rectangle buttonRect, bool buttonState, const char* label, Color color);
// Ініціалізація кнопок вже здійснюється через масиви
void buttons_init(void);
// Виклик малювання кнопок
void DrawButtons(void);
// Обробка натискання на кнопки
void HandleButtonClicks(void);
// Функція для обробки натискання кнопок
void HandleButtonClick(Rectangle buttonRect, bool* buttonState);


#endif // BUTTON_H

