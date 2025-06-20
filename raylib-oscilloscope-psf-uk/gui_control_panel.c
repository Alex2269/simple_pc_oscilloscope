// gui_control_panel.c

#include "raylib.h"
#include "main.h" // для OscData, MAX_CHANNELS
#include "gui_control_panel.h"
#include "button.h"
#include "guicheckbox.h"
#include "sliders.h"
#include "knob_gui.h"

#include <stddef.h>
#include <stdio.h>

extern int fontSize;
extern int LineSpacing;
extern Font font;

#include "psf_font.h"  // Ваш заголовок із парсером PSF
extern PSF_Font psfFont;  // Оголошення глобальної змінної шрифту

#include "gui_dropdown.h"

// Статичний масив для стану відкриття dropdown для кожного каналу
static bool dropdownOpen[MAX_CHANNELS] = { false };
static const char *dropdownItems[] = { "Rising", "Falling", "Auto" };

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

    // Заголовок панелі
    // DrawText("Control Panel", panelX + 10, panelY + 10, 20, WHITE);
    DrawPSFTextUTF8(psfFont, panelX + 20, panelY + 10, "Панель Керування", 2, WHITE);


    // Кнопки вибору активного каналу з кольорами
    for (int i = 0; i < MAX_CHANNELS; i++) {
        Rectangle btnRect = { panelX + 20 + i * 80, panelY + 60, 60, 30 };
        Color btnColor = (oscData->active_channel == i) ? channel_colors[i] : Fade(channel_colors[i], 0.5f);

        if (    Gui_Button(btnRect, TextFormat("CH%d", i + 1), btnColor, GRAY, DARKGRAY, (Color){0,0,0,0} )           ) {
            oscData->active_channel = i;
        }
    }

    // розміри слайдерів
    int W_size = 200;
    int H_size = 30;
    // Відступи для слайдерів
    int sliderX = panelX + 20;
    int sliderY = panelY + 90;
    int spacingY = 100;

    int ch = oscData->active_channel;
    Color activeColor = channel_colors[ch];

    sliderY += 60;

    int knob_radius = 40;
    // Масштабування по вертикалі
    Gui_Knob_Channel(0, sliderX + knob_radius, sliderY,
                     TextFormat("Масштабування CH%d\nпо вертикалі", (int)oscData->active_channel + 1),
                     TextFormat("%0.1f", oscData->channels[ch].scale_y),
                     knob_radius,
                     &oscData->channels[ch].scale_y, 0.1f, 2.0f, true, activeColor);

    // Зміщення по вертикалі
    Gui_Knob_Channel(1, sliderX + 200, sliderY,
                     "Vertical\noffset", TextFormat("%d", (int)oscData->channels[ch].offset_y),
                     knob_radius,
                     &oscData->channels[ch].offset_y, -200.0f, 1200.0f, true, activeColor);

    sliderY += spacingY;

    // Рівень тригера
    Gui_Knob_Channel(2, sliderX + knob_radius, sliderY,
                     "Trigger\nlevel", TextFormat("%0.1f", oscData->channels[ch].trigger_level),
                     knob_radius,
                     &oscData->channels[ch].trigger_level, 0.0f, 1.0f, true, activeColor);

    // Рівень гістерезису
    Gui_Knob_Channel(3, sliderX + 200, sliderY,
                     "Trigger\nhysteresis", TextFormat("%0.1f", oscData->channels[ch].trigger_hysteresis_px),
                     knob_radius,
                     &oscData->channels[ch].trigger_hysteresis_px, -1.0f, 1.0f, true, activeColor);

    sliderY += spacingY;


    // Регулятор частоти оновлення інтерфейсу (мс)
    Gui_Knob_Channel(4, sliderX + knob_radius, sliderY,
                     "Refresh\nRate (ms)", TextFormat("%d", (int)oscData->refresh_rate_ms),
                     knob_radius,
                     &oscData->refresh_rate_ms, 5.0f, 50.0f, true, DARKGRAY);

    // **Пояснення:**
    // - Мета отримати інкремент слайдеру 5
    // - `GuiSlider` повертає значення з плаваючою точкою.
    // - Ми додаємо 2.5 для коректного округлення при діленні на 5.
    // - Приводимо до int для цілочисельного ділення і множимо назад на 5 — отримуємо крок 5.
    // - Таким чином користувач бачить і змінює значення слайдера з кроком 5.
    oscData->refresh_rate_ms = ((int)(oscData->refresh_rate_ms + 2.5f) / 5) * 5;

    static float old_refresh_rate_ms;
    char command[24] = "Rate:";  // достатній розмір буфера

    if (old_refresh_rate_ms != oscData->refresh_rate_ms)
    {
        old_refresh_rate_ms = oscData->refresh_rate_ms;
        send_command(oscData, command, sizeof(command), (int)oscData->refresh_rate_ms);
    }

    // Регулятор горизонтального зміщення тригера
    Gui_Knob_Channel(5, sliderX + 200, sliderY,
                     "Trigger\noffset X", TextFormat("%d", (int)oscData->trigger_offset_x),
                     knob_radius,
                     &oscData->trigger_offset_x, 50, 600, true, DARKGRAY);

    sliderY += spacingY / 2 + 25;

    Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30},
                &oscData->channels[ch].trigger_active,
                 "Activate\ntrigger", "Trigger", activeColor);

    Rectangle dropdownBounds = { sliderX + 200, sliderY, 100, 30 };

    int newSelection = Gui_Dropdown(dropdownBounds, dropdownItems, 3,
                                   oscData->channels[ch].trigger_edge,
                                   "Тип\nфронту\nтригера",
                                   &dropdownOpen[ch],
                                   activeColor);

    if (newSelection != oscData->channels[ch].trigger_edge) {
        oscData->channels[ch].trigger_edge = newSelection;

        // Відправка команди на пристрій
        char cmd[32] = "TriggerEdge:";
        send_command(oscData, cmd, sizeof(cmd), newSelection);
    }

    sliderY += spacingY - 20;

    // Перемикач тестового сигналу
    Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30},
                &oscData->test_signal,
                "Тестовий\nсигнал", NULL, DARKGRAY);

    static bool test_signal;
    char command2[24] = "Test signal:";  // достатній розмір буфера

    if (test_signal != oscData->test_signal)
    {
        test_signal = oscData->test_signal;
        send_command(oscData, command2, sizeof(command2), (int)oscData->test_signal);
    }

    // sliderY += spacingY / 2;

    // Перемикач перемотки сигналу
    Gui_CheckBox((Rectangle){sliderX + 75, sliderY, 30, 30},
                &oscData->movement_signal,
                "Прокручування\nсигналу\nMovement", NULL, DARKGRAY);

    // sliderY += spacingY / 2;

    // Перемикач реверсу напрямку малювання сигналу
    Gui_CheckBox((Rectangle){sliderX+150, sliderY, 30, 30},
                &oscData->reverse_signal,
                "Реверс\nсигналу", NULL, DARKGRAY);

    // Автоматичне підключення (тимчасово вимкнено)
    /* Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30}, "Auto Connect", &oscData->auto_connect);
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

    /*
    int parse;
    if (strncmp(str, "Test signal:", 12) == 0)
    {
        parse = atoi(str + 12);
        printf("parse %d\n", parse);
    }
    */

    printf("%s", str);
    // printf("new_rate: %d\n\n", new_rate);
}

