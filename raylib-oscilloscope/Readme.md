#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Для usleep
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h" // Бібліотека raygui
#include "rs232.h"  // Припускаємо, що у вас є бібліотека RS232

// --- Структура для даних осцилографа ---
typedef struct OscData {
    int comport_number;
    int ray_speed; // Затримка читання в мікросекундах
    int adc_tmp_a;
    int adc_tmp_b;
    float channel_a_history[500];
    float channel_b_history[500];
    int history_index;
    float scale_y_a; // Масштаб по осі Y для каналу A
    float scale_y_b; // Масштаб по осі Y для каналу B
    float offset_y_a;  // Зміщення по осі Y для каналу A (змінено на float)
    float offset_y_b;  // Зміщення по осі Y для каналу B (змінено на float)
    float refresh_rate_ms; // Частота оновлення в мілісекундах (змінено на float)
    bool auto_connect;   // Чи намагатись автоматично підключатись до COM-порту
    char com_port_name_input[20]; // Збільшено розмір для '/dev/ttyACM0'
    bool com_port_name_edit_mode; // Режим редагування для вводу COM-порту
} OscData;

// --- Прототипи функцій ---
int filter_signal_a(int raw_value);
int filter_signal_b(int raw_value);
void read_usb_device(OscData *data);
void draw_oscilloscope_grid(int screenWidth, int screenHeight);

// --- Функція читання даних з USB-CDC (COM-порту) ---
void read_usb_device(OscData *data) {
    static char str[256];
    static char buffer_tmp_a[5], buffer_tmp_b[5];

    if (data->comport_number < 0) {
        return;
    }

    int bytes_read = RS232_PollComport(data->comport_number, (unsigned char*)str, sizeof(str) - 1);
    str[bytes_read] = '\0';

    usleep(data->ray_speed);

    if (bytes_read > 0) {
        char *ptr_a = strstr(str, "A:");
        char *ptr_b = strstr(str, "B:");

        if (ptr_a && ptr_b) {
            if (strlen(ptr_a) >= 2 + 4) {
                strncpy(buffer_tmp_a, ptr_a + 2, 4);
                buffer_tmp_a[4] = '\0';
                data->adc_tmp_a = atoi(buffer_tmp_a);
            }

            if (strlen(ptr_b) >= 2 + 4) {
                strncpy(buffer_tmp_b, ptr_b + 2, 4);
                buffer_tmp_b[4] = '\0';
                data->adc_tmp_b = atoi(buffer_tmp_b);
            }

            data->adc_tmp_a = filter_signal_a(data->adc_tmp_a);
            data->adc_tmp_b = filter_signal_b(data->adc_tmp_b);

            data->channel_a_history[data->history_index] = (float)data->adc_tmp_a;
            data->channel_b_history[data->history_index] = (float)data->adc_tmp_b;
            data->history_index = (data->history_index + 1) % 500;
        } else {
            data->adc_tmp_a = 0;
            data->adc_tmp_b = 0;
        }
    } else {
        data->adc_tmp_a = 0;
        data->adc_tmp_b = 0;
    }
}

// --- Приклад простих функцій фільтрації (можете замінити на свої) ---
int filter_signal_a(int raw_value) {
    static int history[10];
    static int index = 0;
    static int sum = 0;

    sum = sum - history[index] + raw_value;
    history[index] = raw_value;
    index = (index + 1) % 10;

    return sum / 10;
}

int filter_signal_b(int raw_value) {
    return raw_value;
}

// --- Функція для малювання сітки осцилографа ---
void draw_oscilloscope_grid(int screenWidth, int screenHeight) {
    // Горизонтальні лінії
    for (int y = 0; y < screenHeight; y += 50) {
        DrawLine(0, y, screenWidth, y, LIGHTGRAY);
    }
    // Вертикальні лінії
    for (int x = 0; x < screenWidth; x += 50) {
        DrawLine(x, 0, x, screenHeight, LIGHTGRAY);
    }
    // Центральні лінії
    DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, GRAY);
    DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, GRAY);
}

// --- Основна функція програми ---
int main(void) {
    // --- Ініціалізація Raylib ---
    const int screenWidth = 1000;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Raylib Oscilloscope with Controls");
    SetTargetFPS(60);

    // --- Ініціалізація даних осцилографа ---
    OscData oscData = {0};
    oscData.comport_number = -1;
    oscData.ray_speed = 1000;
    oscData.scale_y_a = 0.1f;
    oscData.scale_y_b = 0.1f;
    oscData.offset_y_a = screenHeight / 4.0f; // Змінено на float
    oscData.offset_y_b = screenHeight / 4.0f * 3.0f; // Змінено на float
    oscData.refresh_rate_ms = 10.0f;   // Змінено на float
    oscData.auto_connect = true;
    strcpy(oscData.com_port_name_input, "/dev/ttyACM0"); // Встановлюємо типовий порт для Linux

    // --- Ініціалізація raygui ---
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);

    // --- Змінні для керування COM-портом ---
    // Ми не будемо використовувати фіксований масив com_port_options,
    // оскільки користувач буде вводити ім'я порту.
    // Просто перевіримо номер, який відповідає в rs232.h

    // --- Головний цикл Raylib ---
    float frameTime = 0.0f;
    while (!WindowShouldClose()) {
        // --- Оновлення (логіка програми) ---
        frameTime += GetFrameTime();

        if (frameTime * 1000.0f >= oscData.refresh_rate_ms) { // Використовуємо float для порівняння
            read_usb_device(&oscData);
            frameTime = 0.0f;
        }

        // --- Керування COM-портом (автоматичне підключення) ---
        if (oscData.auto_connect && oscData.comport_number == -1) {
            char mode[] = {'8', 'N', '1', 0};
            int port_idx_to_open = -1;

            // RS232 бібліотека працює з індексами портів (0, 1, 2...).
            // Нам потрібно перетворити рядок "/dev/ttyACM0" на відповідний індекс.
            // У rs232.c є внутрішня функція (rs232_port_names), яка мапить імена на індекси.
            // Оскільки вона не експортується, ми можемо спробувати відкрити кілька індексів
            // або покладатися на те, що користувач введе правильний індекс або відоме ім'я.
            // Для "/dev/ttyACM0" це зазвичай індекс 0, 1, 2 і т.д. в залежності від порядку підключення.
            // Найпростіший спосіб: якщо введено "/dev/ttyACM0", спробувати відкрити порт 0,
            // якщо "/dev/ttyACM1" - порт 1, тощо. Це не надійно для всіх систем.
            // Краще використовувати RS232_GetPortname для пошуку або вручну задати індекс.
            // Для прикладу, ми спробуємо знайти індекс з числа в імені порту.
            if (strncmp(oscData.com_port_name_input, "/dev/ttyACM", 11) == 0) {
                port_idx_to_open = atoi(oscData.com_port_name_input + 11); // Витягуємо число
            } else if (strncmp(oscData.com_port_name_input, "COM", 3) == 0) { // Для Windows
                port_idx_to_open = atoi(oscData.com_port_name_input + 3) - 1; // COM1 -> 0, COM2 -> 1
            }

            if (port_idx_to_open != -1) {
                if (RS232_OpenComport(port_idx_to_open, 115200, mode) == 0) {
                    oscData.comport_number = port_idx_to_open;
                    printf("Відкрито COM порт: %s (індекс %d)\n", oscData.com_port_name_input, oscData.comport_number);
                } else {
                    printf("Не вдалося відкрити COM порт: %s\n", oscData.com_port_name_input);
                }
            } else {
                printf("Невірне або непідтримуване ім'я COM порту для автоматичного підключення: %s\n", oscData.com_port_name_input);
            }
        }


        // --- Відображення ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // --- Малювання області осцилографа ---
        int osc_width = screenWidth - 250;
        int osc_height = screenHeight;

        DrawRectangle(0, 0, osc_width, osc_height, BLACK);

        draw_oscilloscope_grid(osc_width, osc_height);

        // Відображення поточних значень
        DrawText(TextFormat("A: %d", oscData.adc_tmp_a), 10, 10, 20, GREEN);
        DrawText(TextFormat("B: %d", oscData.adc_tmp_b), 10, 40, 20, YELLOW);

        // Малювання осцилограм
        for (int i = 0; i < 500 - 1; i++) {
            // Канал A (червоний)
            int prev_idx_a = (oscData.history_index + i) % 500;
            int curr_idx_a = (oscData.history_index + i + 1) % 500;

            Vector2 start_a = { (float)i * (osc_width / 500.0f),
                               oscData.offset_y_a - (oscData.channel_a_history[prev_idx_a] * oscData.scale_y_a) };
            Vector2 end_a = { (float)(i + 1) * (osc_width / 500.0f),
                             oscData.offset_y_a - (oscData.channel_a_history[curr_idx_a] * oscData.scale_y_a) };
            DrawLineV(start_a, end_a, RED);

            // Канал B (синій)
            int prev_idx_b = (oscData.history_index + i) % 500;
            int curr_idx_b = (oscData.history_index + i + 1) % 500;

            Vector2 start_b = { (float)i * (osc_width / 500.0f),
                               oscData.offset_y_b - (oscData.channel_b_history[prev_idx_b] * oscData.scale_y_b) };
            Vector2 end_b = { (float)(i + 1) * (osc_width / 500.0f),
                             oscData.offset_y_b - (oscData.channel_b_history[curr_idx_b] * oscData.scale_y_b) };
            DrawLineV(start_b, end_b, BLUE);
        }

        // --- Малювання панелі керування raygui ---
        int panel_x = screenWidth - 240;
        int current_y = 10;

        GuiSetStyle(DEFAULT, TEXT_SIZE, 18);

        // Стан COM-порту
        GuiLabel((Rectangle){ panel_x, current_y, 220, 25 }, TextFormat("COM Port: %s", (oscData.comport_number != -1) ? oscData.com_port_name_input : "Not Connected"));
        current_y += 30;

        // Ввід імені COM-порту
        GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, 0); // 0 = TEXT_ALIGN_LEFT
        if (GuiTextBox((Rectangle){ panel_x, current_y, 150, 30 }, oscData.com_port_name_input, sizeof(oscData.com_port_name_input) - 1, oscData.com_port_name_edit_mode)) {
            oscData.com_port_name_edit_mode = !oscData.com_port_name_edit_mode;
        }
        if (GuiButton((Rectangle){ panel_x + 160, current_y, 60, 30 }, "Connect")) {
            char mode[] = {'8', 'N', '1', 0};
            int port_to_open_idx = -1;

            if (strncmp(oscData.com_port_name_input, "/dev/ttyACM", 11) == 0) {
                port_to_open_idx = atoi(oscData.com_port_name_input + 11);
            } else if (strncmp(oscData.com_port_name_input, "COM", 3) == 0) {
                port_to_open_idx = atoi(oscData.com_port_name_input + 3) - 1;
            } else {
                printf("Невірне або непідтримуване ім'я COM порту: %s\n", oscData.com_port_name_input);
            }


            if (oscData.comport_number != -1) {
                RS232_CloseComport(oscData.comport_number);
                oscData.comport_number = -1;
            }

            if (port_to_open_idx != -1) {
                if (RS232_OpenComport(port_to_open_idx, 115200, mode) == 0) {
                    oscData.comport_number = port_to_open_idx;
                    printf("Відкрито COM порт: %s (індекс %d)\n", oscData.com_port_name_input, oscData.comport_number);
                } else {
                    printf("Не вдалося відкрити COM порт: %s\n", oscData.com_port_name_input);
                }
            }
        }
        current_y += 40;

        if (GuiButton((Rectangle){ panel_x, current_y, 220, 30 }, "Disconnect")) {
            if (oscData.comport_number != -1) {
                RS232_CloseComport(oscData.comport_number);
                oscData.comport_number = -1;
                printf("COM порт відключено.\n");
            }
        }
        current_y += 40;

        GuiSetStyle(DEFAULT, TEXT_SIZE, 16);

        // Повзунок масштабу каналу A
        GuiLabel((Rectangle){ panel_x, current_y, 220, 20 }, "Channel A Scale (V/div)");
        current_y += 20;
        GuiSliderBar((Rectangle){ panel_x, current_y, 220, 20 }, NULL, TextFormat("%.2f", oscData.scale_y_a), &oscData.scale_y_a, 0.01f, 0.5f);
        current_y += 30;

        // Повзунок зміщення каналу A
        GuiLabel((Rectangle){ panel_x, current_y, 220, 20 }, "Channel A Offset");
        current_y += 20;
        GuiSliderBar((Rectangle){ panel_x, current_y, 220, 20 }, NULL, TextFormat("%d", (int)oscData.offset_y_a), &oscData.offset_y_a, 0, screenHeight);
        current_y += 30;

        // Повзунок масштабу каналу B
        GuiLabel((Rectangle){ panel_x, current_y, 220, 20 }, "Channel B Scale (V/div)");
        current_y += 20;
        GuiSliderBar((Rectangle){ panel_x, current_y, 220, 20 }, NULL, TextFormat("%.2f", oscData.scale_y_b), &oscData.scale_y_b, 0.01f, 0.5f);
        current_y += 30;

        // Повзунок зміщення каналу B
        GuiLabel((Rectangle){ panel_x, current_y, 220, 20 }, "Channel B Offset");
        current_y += 20;
        GuiSliderBar((Rectangle){ panel_x, current_y, 220, 20 }, NULL, TextFormat("%d", (int)oscData.offset_y_b), &oscData.offset_y_b, 0, screenHeight);
        current_y += 30;

        // Повзунок частоти оновлення
        GuiLabel((Rectangle){ panel_x, current_y, 220, 20 }, "Refresh Rate (ms)");
        current_y += 20;
        GuiSliderBar((Rectangle){ panel_x, current_y, 220, 20 }, NULL, TextFormat("%d", (int)oscData.refresh_rate_ms), &oscData.refresh_rate_ms, 1, 100);
        current_y += 30;

        // Перемикач "Auto Connect"
        GuiCheckBox((Rectangle){ panel_x, current_y, 20, 20 }, "Auto Connect", &oscData.auto_connect);
        current_y += 30;

        EndDrawing();
    }

    // --- Закриття COM порту ---
    if (oscData.comport_number != -1) {
        RS232_CloseComport(oscData.comport_number);
        printf("COM порт %d закрито.\n", oscData.comport_number);
    }

    // --- Завершення Raylib ---
    CloseWindow();

    return 0;
}
