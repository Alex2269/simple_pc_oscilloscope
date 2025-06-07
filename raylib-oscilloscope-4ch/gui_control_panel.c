// gui_control_panel.c

#include "raylib.h"
#include "main.h" // для OscData, MAX_CHANNELS
#include "raygui.h"
#include "gui_control_panel.h"
#include <stddef.h>
#include <stdio.h>

extern int fontSize;
extern int LineSpacing;
extern Font font;

void send_command(OscData *data, char* command, size_t buffer_size, int number);
// void write_usb_device(OscData *data, unsigned char* str);
void write_usb_device(OscData *data, unsigned char* str, size_t len);

// Функція відображає панель керування параметрами осцилографа
// Масив кольорів каналів
static Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, BLUE };

// Масив кольорів каналів
// static Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, BLUE };

void gui_control_panel(OscData *oscData, int screenWidth, int screenHeight) {
    // Позиції і розміри панелі
    int panelX = screenWidth - 350;
    int panelY = 10;
    int panelWidth = 340;
    int panelHeight = screenHeight - 20;

    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(DARKGRAY, 0.9f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, GRAY);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    // Заголовок панелі
    DrawText("Control Panel", panelX + 10, panelY + 10, 20, WHITE);

    // Кнопки вибору активного каналу з кольорами
    for (int i = 0; i < MAX_CHANNELS; i++) {
        Rectangle btnRect = { panelX + 10 + i * 80, panelY + 50, 80, 30 };
        Color btnColor = (oscData->active_channel == i) ? channel_colors[i] : Fade(channel_colors[i], 0.5f);

        // Встановлюємо кольори кнопки (BUTTON = 2)
        GuiSetStyle(2, 0, ColorToInt(btnColor)); // BASE_COLOR_NORMAL
        GuiSetStyle(2, 1, ColorToInt(btnColor)); // BORDER_COLOR_NORMAL
        GuiSetStyle(2, 2, 0xFFFFFFFF);           // TEXT_COLOR_NORMAL

        if (GuiButton(btnRect, TextFormat("CH%d", i + 1))) {
            oscData->active_channel = i;
        }
    }
    // Повертаємо стилі кнопок до дефолтних
    GuiSetStyle(2, 0, 0x363636ff); // BASE_COLOR_NORMAL
    GuiSetStyle(2, 1, 0x000000ff); // BORDER_COLOR_NORMAL
    GuiSetStyle(2, 2, 0xFFFFFFFF); // TEXT_COLOR_NORMAL

    // розміри слайдерів
    int W_size = 200;
    int H_size = 30;
    // Відступи для слайдерів
    int sliderX = panelX + 20;
    int sliderY = panelY + 80;
    int spacingY = 70;

    int ch = oscData->active_channel;
    Color activeColor = channel_colors[ch];

    // Встановлюємо кольори для надписів і слайдерів активного каналу
    GuiSetStyle(1, 2, ColorToInt(activeColor)); // LABEL, TEXT_COLOR_NORMAL
    GuiSetStyle(6, 3, ColorToInt(activeColor)); // SLIDER, SLIDER_COLOR
    GuiSetStyle(6, 4, ColorToInt(activeColor)); // SLIDER, SLIDER_BORDER_COLOR
    GuiSetStyle(6, 5, 2);                      // SLIDER, SLIDER_BORDER_WIDTH

    sliderY += 45;
    DrawTextEx(font, "Vertical scale", (Vector2){sliderX, sliderY - H_size}, fontSize, LineSpacing, activeColor);
    GuiSlider((Rectangle){sliderX, sliderY, W_size, H_size},
              NULL, NULL, &oscData->channels[ch].scale_y, 0.1f, 2.0f);
    DrawTextEx(font, TextFormat(" %0.2f", oscData->channels[ch].scale_y),
               (Vector2){sliderX + W_size, sliderY}, fontSize, LineSpacing, activeColor);

    // Зміщення по вертикалі
    sliderY += spacingY;
    DrawTextEx(font, "Vertical offset", (Vector2){sliderX, sliderY - H_size}, fontSize, LineSpacing, activeColor);
    GuiSlider((Rectangle){sliderX, sliderY, W_size, H_size},
              NULL, NULL, &oscData->channels[ch].offset_y, -200.0f, 1200.0f);
    DrawTextEx(font, TextFormat(" %0.2f", oscData->channels[ch].offset_y),
               (Vector2){sliderX + W_size, sliderY}, fontSize, LineSpacing, activeColor);

    // Рівень тригера
    sliderY += spacingY;
    DrawTextEx(font, "Trigger level", (Vector2){sliderX, sliderY - H_size}, fontSize, LineSpacing, activeColor);
    GuiSlider((Rectangle){sliderX, sliderY, W_size, H_size},
              NULL, NULL, &oscData->channels[ch].trigger_level, 0.0f, 1.0f);
    DrawTextEx(font, TextFormat(" %0.2f", oscData->channels[ch].trigger_level),
               (Vector2){sliderX + W_size, sliderY}, fontSize, LineSpacing, activeColor);

    // Прапорець активації тригера (CHECKBOX = 4)
    GuiSetStyle(4, 1, ColorToInt(activeColor)); // CHECKBOX, BORDER_COLOR_NORMAL
    GuiSetStyle(4, 6, ColorToInt(activeColor)); // CHECKBOX, CHECK_COLOR

    sliderY += spacingY/2;
    GuiCheckBox((Rectangle){sliderX, sliderY, 20, 20},
               NULL, &oscData->channels[ch].trigger_active);
    DrawTextEx(font, "  Trigger Active",
               (Vector2){sliderX, sliderY, 20, 20}, fontSize, LineSpacing, activeColor);

    // Повертаємо стилі чекбокса до дефолтних
    GuiSetStyle(4, 1, 0x000000ff); // BORDER_COLOR_NORMAL
    GuiSetStyle(4, 6, 0xFFFFFFFF); // CHECK_COLOR

    // Повертаємо стилі надписів і слайдерів до дефолтних
    GuiSetStyle(1, 2, 0xFFFFFFFF); // LABEL, TEXT_COLOR_NORMAL
    GuiSetStyle(6, 3, 0xAAAAAAFF); // SLIDER, SLIDER_COLOR
    GuiSetStyle(6, 4, 0xAAAAAAFF); // SLIDER, SLIDER_BORDER_COLOR
    GuiSetStyle(6, 5, 1);          // SLIDER, SLIDER_BORDER_WIDTH

    // Повзунок частоти оновлення інтерфейсу (мс)
    sliderY += spacingY;
    DrawTextEx(font, "Refresh Rate (ms)", (Vector2){sliderX, sliderY - H_size}, fontSize, LineSpacing, WHITE);

    GuiSlider((Rectangle){sliderX, sliderY, W_size, H_size},
              NULL, NULL, &oscData->refresh_rate_ms, 5.0f, 50.0f);

    static float old_refresh_rate_ms;
    char command[24] = "Rate:";  // достатній розмір буфера

    // **Пояснення:**
    // - Мета отримати інкремент слайдеру 5
    // - `GuiSlider` повертає значення з плаваючою точкою.
    // - Ми додаємо 2.5 для коректного округлення при діленні на 5.
    // - Приводимо до int для цілочисельного ділення і множимо назад на 5 — отримуємо крок 5.
    // - Таким чином користувач бачить і змінює значення слайдера з кроком 5.
    oscData->refresh_rate_ms = ((int)(oscData->refresh_rate_ms + 2.5f) / 5) * 5;

    if (old_refresh_rate_ms != oscData->refresh_rate_ms)
    {
        old_refresh_rate_ms = oscData->refresh_rate_ms;
        send_command(oscData, command, sizeof(command), (int)oscData->refresh_rate_ms);
    }
    DrawTextEx(font, TextFormat(" %.1f", oscData->refresh_rate_ms),
               (Vector2){sliderX + W_size, sliderY}, fontSize, LineSpacing, WHITE);

    sliderY += spacingY;
    // Повзунок горизонтального зміщення тригера
    DrawTextEx(font, "Trigger offset X", (Vector2){sliderX, sliderY - H_size}, fontSize, LineSpacing, WHITE);

    GuiSlider((Rectangle){sliderX, sliderY, W_size, H_size},
              NULL, NULL, &oscData->trigger_offset_x, 50, 600);
    DrawTextEx(font, TextFormat(" %d", (int)oscData->trigger_offset_x),
               (Vector2){sliderX + W_size, sliderY}, fontSize, LineSpacing, WHITE);

    sliderY += spacingY;
    // Перемикач реверсу напрямку малювання сигналу
    GuiCheckBox((Rectangle){sliderX, sliderY, 20, 20},
                NULL, &oscData->reverse_signal);
    DrawTextEx(font, "  Reverse Signal",
               (Vector2){sliderX, sliderY, 20, 20}, fontSize, LineSpacing, WHITE);

    // Автоматичне підключення (тимчасово вимкнено)
    /* GuiCheckBox((Rectangle){sliderX, sliderY, 20, 20}, "Auto Connect", &oscData->auto_connect);
    sliderY += spacingY; */
}

void send_command(OscData *data, char* command, size_t buffer_size, int number)
{
    //snprintf(command, command_size, "Rate: %d", number);
    size_t len = strlen(command);

    // Перед викликом snprintf добре перевірити,
    // що buffer_size > len щоб уникнути переповнення:
    if (buffer_size > len) {
        snprintf(command + len, buffer_size - len, "%d", number);
    } else {
        // Обробка помилки: недостатньо місця в буфері
        fprintf(stderr, "Buffer overflow prevented in send_command\n");
        return;
    }

    if (buffer_size > len) {
        command[len] = '\0';
    }

    /*
    // Якщо потрібно, щоб між текстом і числом був пробіл,
    // двокрапка або інший символ, додай його перед snprintf:
    if (buffer_size > len) {
        command[len] = ':';  // або ':', або інший символ
        command[len + 1] = ' ';  // або ':', або інший символ
        command[len + 2] = '\0';
        len += 2;
    }
    */

    snprintf(command + len, buffer_size - len, "%d\n", number);
    // printf("command: %s\n", command);
    // printf("number: %d\n", number);


    len = strlen(command);
    write_usb_device(data, command, len);
}

void write_usb_device(OscData *data, unsigned char* str, size_t len)
{
    // RS232_cputs(data->comport_number, str); // не працює ???
    // size_t len = strlen(str);
    RS232_SendBuf(data->comport_number, str, len);
    // printf("%s\n", str);

    // Код перевірки парсингу
    int new_rate;
    if (strncmp(str, "Rate:", 5) == 0)
    {
        new_rate = atoi(str + 5);
        // set_adc_sampling_rate(new_rate);
    }

    /*
    if (strncmp(str, "Rate:", 5) == 0)
    {
        new_rate = atoi(str + 6);
        // set_adc_sampling_rate(new_rate);
        // printf("new_rate %d \n", new_rate);
    }*/

    printf("%s\n", str);
    printf("new_rate: %d\n", new_rate);
}

