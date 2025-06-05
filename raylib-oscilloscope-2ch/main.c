// main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // usleep
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "rs232.h"
#include "LoadFontUnicode.h"
#include "draw_grid.h"

#include "main.h"
#include "parse_data.h"
#include "gui_control_panel.h"
#include "draw_signal.h"
#include "generate_test_signals.h"
#include <stdio.h>

int fontSize = 24;
int LineSpacing = 0;
Font font;

// Фільтр ковзного середнього для сигналів
int filter_signal_a(int raw_value) {
    static int history[4];
    static int index = 0;
    static int sum = 0;

    sum = sum - history[index] + raw_value;
    history[index] = raw_value;
    index = (index + 1) % 4;

    return sum / 4;
}

int filter_signal_b(int raw_value) {
    static int history[4];
    static int index = 0;
    static int sum = 0;

    sum = sum - history[index] + raw_value;
    history[index] = raw_value;
    index = (index + 1) % 4;

    return sum / 4;
}

// Читання даних з COM-порту, парсинг ADC значень і запис у історію
void read_usb_device(OscData *data) {
    static char str[256];
    static char buffer_tmp_a[5], buffer_tmp_b[5];

    if (data->comport_number < 0) {
        data->adc_tmp_a = 0;
        data->adc_tmp_b = 0;
        return;
    }

    int bytes_read = RS232_PollComport(data->comport_number, (unsigned char*)str, sizeof(str) - 1);
    str[bytes_read] = '\0';

    usleep(data->ray_speed);

    // Код перевірки парсингу
    parse_data(str, &data->adc_tmp_a, &data->adc_tmp_b);

    // Застосування фільтрів ковзного середнього
    data->adc_tmp_a = filter_signal_a(data->adc_tmp_a);
    data->adc_tmp_b = filter_signal_b(data->adc_tmp_b);

    const int workspace = 550;
    // Масштабування ADC значень у координати для відображення
    data->adc_tmp_a = ((float)data->adc_tmp_a / 4095) * workspace;
    data->adc_tmp_b = ((float)data->adc_tmp_b / 4095) * workspace;

    // Запис у циклічний буфер історії
    data->channel_a_history[data->history_index] = (float)data->adc_tmp_a;
    data->channel_b_history[data->history_index] = (float)data->adc_tmp_b;
    data->history_index = (data->history_index + 1) % 500;
}

// void write_usb_device(OscData *data, unsigned char* str)
// {
//     RS232_cputs(data->comport_number, str);
// }

// Пошук індексу тригера з гістерезисом (перетин рівня знизу вгору)
int find_trigger_index_with_hysteresis(float *history, int history_index, float trigger_level, float hysteresis, int history_size) {
    bool passed_lower = false;
    for (int i = 0; i < history_size - 1; i++) {
        int idx = (history_index + i) % history_size;
        float val = history[idx] / 550.0f; // Нормалізація значення
        if (!passed_lower) {
            if (val < trigger_level - hysteresis) passed_lower = true;
        } else {
            if (val >= trigger_level + hysteresis) return i;
        }
    }
    return 0; // Якщо тригер не знайдено, повертаємо 0
}

int main(void) {
    const int screenWidth = 1000;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Raylib Oscilloscope with Trigger and Scaling");
    font = LoadFontUnicode("bold.ttf", fontSize, LineSpacing);

    SetTargetFPS(60);

    OscData oscData = {0};
    oscData.comport_number = -1;
    oscData.ray_speed = 1000;
    oscData.scale_y_a = 0.50f;
    oscData.scale_y_b = 0.50f;
    oscData.offset_y_a = screenHeight / 4.0f * 2.5f;
    oscData.offset_y_b = screenHeight / 4.0f * 3.5f;
    oscData.refresh_rate_ms = 20.0f;
    oscData.auto_connect = true;
    strcpy(oscData.com_port_name_input, "");
    oscData.trigger_level_a = 0.5f;
    oscData.trigger_level_b = 0.5f;
    oscData.trigger_index_a = 0;
    oscData.trigger_index_b = 0;
    oscData.trigger_active_a = true;
    oscData.trigger_active_b = false;

    // Ініціалізація нових полів для плавного оновлення тригера і затримки
    oscData.trigger_index_a_smooth = 0.0f;
    oscData.trigger_index_b_smooth = 0.0f;
    oscData.trigger_locked_a = false;
    oscData.trigger_locked_b = false;
    oscData.frames_since_trigger_a = 0;
    oscData.frames_since_trigger_b = 0;
    oscData.trigger_offset_x = 300.f;

    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 4);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, 0);

    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, 0x888888FF);

    GuiSetStyle(TEXTBOX, TEXT_COLOR_NORMAL, 0xBBBBBBFF);
    GuiSetStyle(TEXTBOX, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(TEXTBOX, TEXT_COLOR_PRESSED, 0xFFFFFFFF);

    // Автоматичне відкриття COM-порту (спрощено)
    int comport_to_find = 0;
    char mode[] = {'8','N','1',0};
    bool port_found = false;

    while (comport_to_find < 38) {
        if (RS232_OpenComport(comport_to_find, 115200, mode, 0) == 0) {
            oscData.comport_number = comport_to_find;
            printf("Автоматично відкрито COM порт: %d\n", oscData.comport_number);
            port_found = true;
            if (comport_to_find == 24) {
                strcpy(oscData.com_port_name_input, "/dev/ttyACM0");
            } else {
                sprintf(oscData.com_port_name_input, "Port %d", comport_to_find);
            }
            break;
        }
        comport_to_find++;
        if (comport_to_find == 37) {
            sleep(1);
        }
    }

    if (!port_found) {
        printf("Не вдалося автоматично відкрити жоден COM порт.\n");
        strcpy(oscData.com_port_name_input, "/dev/ttyACM0");
    }

    float frameTime = 0.0f;

    while (!WindowShouldClose()) {
        frameTime += GetFrameTime();

        // Читаємо нові дані з пристрою з заданою частотою оновлення
        if (frameTime * 1000.0f >= oscData.refresh_rate_ms) {
            read_usb_device(&oscData);
            frameTime = 0.0f;
        }

        // Параметри гістерезису для тригера
        float hysteresis = 0.05f;

        // Обробка тригера каналу A
        if (oscData.trigger_active_a) {
            int new_trigger_index_a = find_trigger_index_with_hysteresis(
                oscData.channel_a_history,
                oscData.history_index,
                oscData.trigger_level_a,
                hysteresis,
                500);

            // Плавне згладжування індексу тригера
            float alpha = 0.1f;
            oscData.trigger_index_a_smooth = alpha * new_trigger_index_a + (1 - alpha) * oscData.trigger_index_a_smooth;

            // Оновлення індексу і блокування малювання після накопичення даних
            if ((int)(oscData.trigger_index_a_smooth + 0.5f) != oscData.trigger_index_a) {
                oscData.trigger_index_a = (int)(oscData.trigger_index_a_smooth + 0.5f);
                oscData.trigger_locked_a = false;
                oscData.frames_since_trigger_a = 0;
            } else {
                if (!oscData.trigger_locked_a) {
                    oscData.frames_since_trigger_a++;
                    if (oscData.frames_since_trigger_a > 100) { // Затримка 100 кадрів
                        oscData.trigger_locked_a = true;
                    }
                }
            }
        } else {
            oscData.trigger_index_a = 0;
            oscData.trigger_locked_a = true;
        }

        // Аналогічна обробка тригера каналу B (якщо потрібно)
        if (oscData.trigger_active_b) {
            int new_trigger_index_b = find_trigger_index_with_hysteresis(
                oscData.channel_b_history,
                oscData.history_index,
                oscData.trigger_level_b,
                hysteresis,
                500);

            float alpha = 0.1f;
            oscData.trigger_index_b_smooth = alpha * new_trigger_index_b + (1 - alpha) * oscData.trigger_index_b_smooth;

            if ((int)(oscData.trigger_index_b_smooth + 0.5f) != oscData.trigger_index_b) {
                oscData.trigger_index_b = (int)(oscData.trigger_index_b_smooth + 0.5f);
                oscData.trigger_locked_b = false;
                oscData.frames_since_trigger_b = 0;
            } else {
                if (!oscData.trigger_locked_b) {
                    oscData.frames_since_trigger_b++;
                    if (oscData.frames_since_trigger_b > 100) {
                        oscData.trigger_locked_b = true;
                    }
                }
            }
        } else {
            oscData.trigger_index_b = 0;
            oscData.trigger_locked_b = true;
        }

        static bool dragging_trigger_line = false;
        static float trigger_x_pos = 300.f; // Початкова позиція тригера (можна взяти з oscData.trigger_offset_x)

        Vector2 mousePos = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Якщо клікнули близько до лінії тригера (наприклад, в межах 5 пікселів)
            if (fabsf(mousePos.x - trigger_x_pos) < 5.0f) {
                dragging_trigger_line = true;
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            dragging_trigger_line = false;
        }

        if (dragging_trigger_line) {
            // Обмежуємо позицію лінії в межах екрану (або осцилографа)
            trigger_x_pos = mousePos.x;
            if (trigger_x_pos < 10) trigger_x_pos = 10;
            if (trigger_x_pos > screenWidth - 10) trigger_x_pos = screenWidth - 10;

            // Оновлюємо значення у структурі, якщо потрібно
            oscData.trigger_offset_x = trigger_x_pos;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        int osc_width = screenWidth - 360;
        int osc_height = screenHeight;

        // Фон осцилографа
        DrawRectangle(0, 0, osc_width, osc_height, BLACK);
        draw_grid(osc_width, osc_height);

        // Відображення поточних значень ADC
        DrawTextEx(font, TextFormat("A: %d", oscData.adc_tmp_a), (Vector2){10, 10}, fontSize, 2, YELLOW);
        DrawTextEx(font, TextFormat("B: %d", oscData.adc_tmp_b), (Vector2){10, 40}, fontSize, 2, GREEN);

        // --- Тригерний блок початок ---
        // Обчислюємо позиції по Y для рівнів тригера каналів A і B
        float trigger_level_y_a = oscData.offset_y_a - (oscData.trigger_level_a * 550.0f) * oscData.scale_y_a;
        float trigger_level_y_b = oscData.offset_y_b - (oscData.trigger_level_b * 550.0f) * oscData.scale_y_b;

        // Напівпрозорі кольори для ліній тригера
        Color yellow = (Color){253, 249, 0, 128}; // Жовтий з прозорістю 50%
        Color green  = (Color){0, 228, 48, 128};  // Зелений з прозорістю 50%

        // Зсув по горизонталі для ліній, щоб не малювати по краях
        int line_x_start = 50;
        int line_x_end = (int)osc_width - 50;

        // Малюємо горизонтальні лінії рівня тригера
        DrawLineEx((Vector2){line_x_start, trigger_level_y_a}, (Vector2){line_x_end, trigger_level_y_a}, 2.0f, yellow);
        DrawLineEx((Vector2){line_x_start, trigger_level_y_b}, (Vector2){line_x_end, trigger_level_y_b}, 2.0f, green);

        // Фіксована позиція тригера — 1/4 ширини осцилографа
        // float trigger_x_pos = osc_width / 4.0f;
        // --- Тригерний блок кінець ---

        // Малюємо вертикальну лінію — позицію тригера
        // DrawLine((int)trigger_x_pos, 0, (int)trigger_x_pos, screenHeight, RED);
        DrawLine((int)oscData.trigger_offset_x, 75, (int)oscData.trigger_offset_x, screenHeight-75, RED);

        // Приклад виклику
        // generate_test_signals(&oscData, 500, 100);
        draw_signal(&oscData, osc_width, 2.0f);

        // --- Виклик GUI панелі керування ---
        gui_control_panel(&oscData, screenWidth, screenHeight);

        EndDrawing();
    }

    if (oscData.comport_number != -1) {
        RS232_CloseComport(oscData.comport_number);
        printf("COM порт %d закрито.\n", oscData.comport_number);
    }

    CloseWindow();

    return 0;
}

