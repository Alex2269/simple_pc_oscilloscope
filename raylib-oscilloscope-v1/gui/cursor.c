// cursor.c
#include "cursor.h"
#include <math.h>
#include <stdio.h>

// Ініціалізація курсора з початковими параметрами
Cursor InitCursor(float startX, float topY, float width, float height, Color color, int minValue, int maxValue) {
    Cursor cursor;
    cursor.x = startX;                // Початкова позиція курсора по X
    cursor.topY = topY;               // Верхня позиція курсора по Y (верхній край)
    cursor.width = width;             // Ширина курсора
    cursor.height = height;           // Висота курсора
    cursor.color = color;             // Колір курсора
    cursor.isDragging = false;        // Чи перетягується курсор зараз
    cursor.minValue = minValue;       // Мінімальне значення, яке може приймати курсор
    cursor.maxValue = maxValue;       // Максимальне значення курсора

    // Обчислення початкового значення курсора пропорційно позиції по X відносно ширини екрану
    cursor.value = minValue + (maxValue - minValue) * ((startX) / GetScreenWidth());

    // Позиція курсора по Y для відображення під курсором (нижче на висоту + тонка лінія)
    cursor.y = topY + height + THIN_LINE;

    return cursor;
}

// Перевірка, чи знаходиться курсор миші над курсором (для початку перетягування)
static bool IsMouseOverCursor(Vector2 mousePos, Cursor cursor) {
    // Розширена область по X (в 2 рази ширше курсора) і по Y - висота курсора
    return (mousePos.x > cursor.x - cursor.width * 2 && mousePos.x < cursor.x + cursor.width * 2 &&
            mousePos.y > cursor.topY && mousePos.y < cursor.topY + cursor.height);
}

// Оновлення позиції курсора під час перетягування
// Перевірка колізії з іншим курсором, щоб вони не накладалися
static void UpdateCursorDrag(Cursor *cursor, Vector2 mousePos, Cursor *otherCursor) {
    cursor->x = mousePos.x;  // Оновлюємо позицію X курсора відповідно до миші

    float minX = cursor->width / 2;             // Мінімальна позиція курсора (щоб не вийти за межі лівого краю)
    float maxX = GetScreenWidth() - cursor->width / 2 - 360;  // Максимальна позиція по X (обмежуємся полем осцилографа)
    float collisionThreshold = cursor->width * 1.5f;    // Мінімальна відстань між курсорами для уникнення накладання

    // Якщо є інший курсор і цей курсор перетягується, перевіряємо зіткнення
    if (otherCursor != NULL && cursor->isDragging && cursor != otherCursor) {
        // Якщо курсор наближається занадто близько зліва до іншого курсора, відсуваємо його
        if (cursor->x < otherCursor->x && cursor->x + collisionThreshold > otherCursor->x) {
            cursor->x = otherCursor->x - collisionThreshold;
        }
        // Якщо курсор наближається занадто близько справа, відсуваємо його
        else if (cursor->x > otherCursor->x && cursor->x - collisionThreshold < otherCursor->x) {
            cursor->x = otherCursor->x + collisionThreshold;
        }
    }

    // Обмеження позиції курсора в межах екрану
    if (cursor->x < minX) cursor->x = minX;
    if (cursor->x > maxX) cursor->x = maxX;

    // Обчислення значення курсора пропорційно позиції по X між мінімумом і максимумом
    cursor->value = cursor->minValue + (cursor->maxValue - cursor->minValue) * ((cursor->x - minX) / (maxX - minX));

    // Гарантія, що значення не виходить за межі
    if (cursor->value < cursor->minValue) cursor->value = cursor->minValue;
    if (cursor->value > cursor->maxValue) cursor->value = cursor->maxValue;

    // Оновлення позиції по Y для відображення (нижче курсора)
    cursor->y = cursor->topY + cursor->height + THIN_LINE;
}

// Оновлення та обробка всіх курсорів: перевірка натискань, перетягування, відпускання миші
void UpdateAndHandleCursors(Cursor *cursors, int count, Vector2 mousePos, bool mouseButtonPressed, bool mouseButtonDown, bool mouseButtonReleased) {
    for (int i = 0; i < count; ++i) {
        Cursor *cursor = &cursors[i];
        // Визначаємо інший курсор для перевірки колізій (якщо їх два)
        Cursor *otherCursor = (count > 1) ? &cursors[(i + 1) % count] : NULL;

        // Якщо натиснули кнопку миші і вона над курсором — починаємо перетягування
        if (mouseButtonPressed && IsMouseOverCursor(mousePos, *cursor)) {
            cursor->isDragging = true;
        }

        // Якщо курсор перетягується і кнопка миші утримується — оновлюємо позицію курсора
        if (cursor->isDragging && mouseButtonDown) {
            UpdateCursorDrag(cursor, mousePos, otherCursor);
        }

        // Якщо кнопку миші відпустили — припиняємо перетягування
        if (mouseButtonReleased) {
            cursor->isDragging = false;
        }
    }
}

// Малювання курсорів, ліній між ними, стрілок і відображення відстані та значень
void DrawCursorsAndDistance(Cursor *cursors, int count, Font font, int fontSize) {

    // Отримуємо позицію миші та стан кнопки миші
    Vector2 mousePos = GetMousePosition();
    bool mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    // Оновлюємо стан курсорів з урахуванням вводу користувача
    UpdateAndHandleCursors(cursors, 2, mousePos, mousePressed, mouseDown, mouseReleased);

    // Малюємо кожен курсор як прямокутник із лінією під ним
    for (int i = 0; i < count; ++i) {
        Cursor cursor = cursors[i];
        DrawRectangle(cursor.x - cursor.width / 2, cursor.topY, cursor.width, cursor.height, cursor.color);
        DrawLine(cursor.x, cursor.topY + cursor.height, cursor.x, cursor.topY + cursor.height + THIN_LINE, LIGHTGRAY);
    }

    // Якщо є два курсори, малюємо лінію між ними, стрілки на кінцях та відстань у пікселях
    if (count >= 2) {
        Cursor cursorA = cursors[0];
        Cursor cursorB = cursors[1];
        float distance = fabs(cursorA.x - cursorB.x); // Відстань по X між курсорами
        float lineY = cursorA.y; // Висота горизонтальної лінії зі стрілками
        Vector2 middlePoint = {(cursorA.x + cursorB.x) / 2, lineY - 15}; // Середина лінії для тексту

        DrawLine(cursorA.x, lineY, cursorB.x, lineY, LIGHTGRAY);

        // Малюємо стрілку для курсора A, напрямок залежить від положення курсора B
        float arrowAOffset = (cursorB.x > cursorA.x) ? ARROW_SIZE : -ARROW_SIZE;
        DrawTriangle(
            (Vector2){cursorA.x, lineY},
            (Vector2){cursorA.x + arrowAOffset, lineY + arrowAOffset / 2},
            (Vector2){cursorA.x + arrowAOffset, lineY - arrowAOffset / 2},
            LIGHTGRAY
        );

        // Малюємо стрілку для курсора B
        float arrowBOffset = (cursorA.x > cursorB.x) ? ARROW_SIZE : -ARROW_SIZE;
        DrawTriangle(
            (Vector2){cursorB.x, lineY},
            (Vector2){cursorB.x + arrowBOffset, lineY + arrowBOffset / 2},
            (Vector2){cursorB.x + arrowBOffset, lineY - arrowBOffset / 2},
            LIGHTGRAY
        );

        // Формуємо рядок з відстанню у пікселях і малюємо текст у центрі лінії
        char distanceText[32];
        sprintf(distanceText, "%i px", (int)distance);
        int textWidth = MeasureText(distanceText, fontSize);
        DrawTextEx(font, distanceText, (Vector2){middlePoint.x - textWidth / 2, middlePoint.y - fontSize - 5}, fontSize, 2, LIGHTGRAY);
    }

    // Вивід тексту зі значеннями кожного курсора у нижній частині екрану
    DrawTextEx(font, TextFormat("Значення курсора A: %i", cursors[0].value), (Vector2){10, GetScreenHeight() - 60}, fontSize, 2, cursors[0].color);
    DrawTextEx(font, TextFormat("Значення курсора B: %i", cursors[1].value), (Vector2){10, GetScreenHeight() - 30}, fontSize, 2, cursors[1].color);
}
