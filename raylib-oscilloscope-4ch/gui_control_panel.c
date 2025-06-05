// gui_control_panel.c

#include "raygui.h"
#include "gui_control_panel.h"
#include <stddef.h>
#include <stdio.h>

void send_command(OscData *data, char* command, size_t buffer_size, int number);
// void write_usb_device(OscData *data, unsigned char* str);
void write_usb_device(OscData *data, unsigned char* str, size_t len);

// Функція відображає панель керування параметрами осцилографа
void gui_control_panel(OscData *oscData, int screenWidth, int screenHeight)
{
    int panel_x = screenWidth - 350;
    int current_y = 5;
    int W_Controls = 290;
    int H_Controls = 18;
    int Vertical_Space = 23;

    DrawRectangle(panel_x - 10, 0, screenWidth - (panel_x - 10), screenHeight, (Color){40, 40, 40, 255});
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    // Вибір активного каналу
    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Select Active Channel:");
    current_y += Vertical_Space;

    // Кнопки вибору каналу
    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        char label[16];
        sprintf(label, "Channel %d", i);
        if (GuiButton((Rectangle){panel_x + i * 70, current_y, 65, 30}, label))
        {
            oscData->active_channel = i;
        }
    }
    current_y += 40;

    // Кнопки активації каналів
    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Activate Channels:");
    current_y += Vertical_Space;

    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        char label[16];
        sprintf(label, "Ch %d", i);
        GuiCheckBox((Rectangle){panel_x + i * 70, current_y, 20, 20}, label, &oscData->channels[i].active);
    }
    current_y += 40;

    // Якщо активний канал неактивний, показуємо повідомлення і не відображаємо повзунки
    ChannelSettings *ch = &oscData->channels[oscData->active_channel];
    if (!ch->active)
    {
        GuiLabel((Rectangle){panel_x, current_y, W_Controls, 30}, "Selected channel is inactive.");
        return;
    }

    // Повзунки для активного каналу
    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Scale (V/div)");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%.2f", ch->scale_y), &ch->scale_y, 0.1f, 1.0f);
    current_y += Vertical_Space;

    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Offset");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%d", (int)ch->offset_y), &ch->offset_y, -screenHeight, screenHeight * 2);
    current_y += Vertical_Space;

    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Trigger Level");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%.2f", ch->trigger_level), &ch->trigger_level, 0.0f, 1.5f);
    current_y += Vertical_Space;

    GuiCheckBox((Rectangle){panel_x, current_y, 20, 20}, "Trigger Active", &ch->trigger_active);
    current_y += 40;

    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xFFFFFFFF);

    // Повзунок частоти оновлення інтерфейсу (мс)
    GuiLabel((Rectangle){ panel_x, current_y, W_Controls, 20 }, "Refresh Rate (ms)");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%.1f", oscData->refresh_rate_ms), &oscData->refresh_rate_ms, 5.0f, 50.0f);
    current_y += Vertical_Space;

    static float old_refresh_rate_ms;
    char command[24] = "Rate:";  // достатній розмір буфера

    // **Пояснення:**
    // - Мета отримати інкремент слайдеру 5
    // - `GuiSliderBar` повертає значення з плаваючою точкою.
    // - Ми додаємо 2.5 для коректного округлення при діленні на 5.
    // - Приводимо до int для цілочисельного ділення і множимо назад на 5 — отримуємо крок 5.
    // - Таким чином користувач бачить і змінює значення слайдера з кроком 5.
    oscData->refresh_rate_ms = ((int)(oscData->refresh_rate_ms + 2.5f) / 5) * 5;

    if (old_refresh_rate_ms != oscData->refresh_rate_ms)
    {
        old_refresh_rate_ms = oscData->refresh_rate_ms;
        send_command(oscData, command, sizeof(command), (int)oscData->refresh_rate_ms);
    }

    // Повзунок горизонтального зміщення тригера
    GuiLabel((Rectangle){ panel_x, current_y, W_Controls, 20 }, "Trigger offset X");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%d", (int)oscData->trigger_offset_x), &oscData->trigger_offset_x, 50, 600);
    current_y += Vertical_Space;

    // Перемикач реверсу напрямку малювання сигналу
    GuiCheckBox((Rectangle){panel_x, current_y, 20, 20}, "Reverse Signal", &oscData->reverse_signal);
    current_y += Vertical_Space;

    // Автоматичне підключення (тимчасово вимкнено)
    /* GuiCheckBox((Rectangle){panel_x, current_y, 20, 20}, "Auto Connect", &oscData->auto_connect);
    current_y += Vertical_Space; */
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




