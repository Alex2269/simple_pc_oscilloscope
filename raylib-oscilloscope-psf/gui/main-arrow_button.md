```c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // usleep
#include <stdbool.h>
#include "raylib.h"

#include "LoadFontUnicode.h"

#include "arrow_button.h"

#include <stdint.h>

#include "psf_font.h"
// #include "GlyphCache.h"

PSF_Font font32;

int fontSize = 24;
int LineSpacing = 0;
Font font;
int spacing = 2; // Відступ між символами, той самий, що передається у DrawPSFText


int main(void) {
    const int screenWidth = 1000;
    const int screenHeight = 600;

    // Встановлюємо прапорець для мультисемплінгу (покращення якості графіки)
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Window");

    // Завантаження PSF шрифту (шлях до вашого файлу)
    font32 = LoadPSFFont("fonts/Uni3-TerminusBold32x16.psf");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(RAYWHITE);

        static int value = 0;
        Rectangle btnInc = { 50, 50, 40, 40 };
        Rectangle btnDec = { 100, 50, 40, 40 };

        if (Gui_ArrowButton(btnInc, font32, ARROW_UP, true, &value, 1, 0, 100, "Інкремент", "Вгору", RED)) {
            // Кнопка інкременту натиснута
        }

        if (Gui_ArrowButton(btnDec, font32, ARROW_DOWN, true, &value, 1, 0, 100, "Декремент", "Вниз", BLUE)) {
            // Кнопка декременту натиснута
        }

        // Малюємо текст праворуч з підтримкою переносу рядків
        DrawPSFText(font32, 100, 200,
                    TextFormat("Значення : %i", value),
                    spacing, BLACK);

        EndDrawing();
    }

    // Після виходу з циклу звільняємо пам'ять шрифту
    // GlyphCache_ClearAllCaches();

    UnloadPSFFont(font32);

    CloseWindow();

    return 0;
}

```

