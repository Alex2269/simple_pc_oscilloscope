// main.c

#include "raylib.h"
#include "LoadFontUnicode.h"
#include "arrow_button.h"

#include "psf_font.h"
// #include "GlyphCache.h"

PSF_Font font32;

int fontSize = 24;
int LineSpacing = 0;
int spacing = 2; // Відступ між символами, той самий, що передається у DrawPSFText


int main(void) {
    InitWindow(400, 200, "Arrow Button with PSF Font and Hold Logic");
    SetTargetFPS(60);

    int value = 0;
    HoldState holdUp = {0};
    HoldState holdDown = {0};

    // Завантаження PSF шрифту (шлях до вашого файлу)
    font32 = LoadPSFFont("fonts/Uni3-TerminusBold32x16.psf");

    Rectangle btnUp = {50, 70, 40, 40};
    Rectangle btnDown = {150, 70, 40, 40};

    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (Gui_ArrowButton(btnUp, font32, ARROW_UP, true, &value, 1, 0, 1000, "Збільшити", "Вгору", BLUE, &holdUp)) {
            // Обробка натискання кнопки "вгору"
        }
        if (Gui_ArrowButton(btnDown, font32, ARROW_DOWN, true, &value, 1, 0, 1000, "Зменшити", "Вниз", RED, &holdDown)) {
            // Обробка натискання кнопки "вниз"
        }

        DrawPSFText(font32, 50, 20, TextFormat("Значення: %d", value), spacing, BLACK);

        EndDrawing();
    }

    // Після виходу з циклу звільняємо пам'ять шрифту
    // GlyphCache_ClearAllCaches();

    UnloadPSFFont(font32);

    CloseWindow();

    return 0;
}


