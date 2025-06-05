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
    // Параметри розміщення елементів панелі
    int panel_x = screenWidth - 350;
    int current_y = 5;
    int W_Controls = 290;
    int H_Controls = 18;
    int Vertical_Space = 23;

    // Фон панелі
    DrawRectangle(panel_x - 10, 0, screenWidth - (panel_x - 10), screenHeight, (Color){40, 40, 40, 255});

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    // Відображення інформації про COM-порт
    GuiSetStyle(DEFAULT, TEXT_SIZE, 10);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 2);
    GuiLabel((Rectangle){panel_x, current_y, W_Controls, 25}, TextFormat("COM Port: %s (Idx: %d)", oscData->com_port_name_input, oscData->comport_number));
    current_y += Vertical_Space;

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 4);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xFFFFFFFF);

    // Текстове поле для введення імені COM-порту користувачем
    if (GuiTextBox((Rectangle){panel_x, current_y, 200, 30}, oscData->com_port_name_input, sizeof(oscData->com_port_name_input) - 1, oscData->com_port_name_edit_mode)) {
      oscData->com_port_name_edit_mode = !oscData->com_port_name_edit_mode;
    }
    current_y += 40;

    // Кнопка "Connect" для відкриття COM-порту
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x404040FF);
    if (GuiButton((Rectangle){panel_x, current_y, 120, 30}, "Connect")) {
      char mode[] = {'8', 'N', '1', 0};
      int port_to_open_idx = -1;

      // Парсинг імені порту у індекс COM-порту
      if (strcmp(oscData->com_port_name_input, "/dev/ttyACM0") == 0) {
        port_to_open_idx = 24;
      } else if (strncmp(oscData->com_port_name_input, "/dev/ttyACM", 11) == 0) {
        port_to_open_idx = atoi(oscData->com_port_name_input + 11);
      } else if (strncmp(oscData->com_port_name_input, "COM", 3) == 0) {
        port_to_open_idx = atoi(oscData->com_port_name_input + 3) - 1;
      } else if (sscanf(oscData->com_port_name_input, "%d", &port_to_open_idx) != 1) {
        printf("Невірне або непідтримуване ім'я COM порту: %s\n", oscData->com_port_name_input);
        port_to_open_idx = -1;
      }

      // Закриваємо попередній порт, якщо відкритий
      if (oscData->comport_number != -1) {
        RS232_CloseComport(oscData->comport_number);
        oscData->comport_number = -1;
      }

      // Відкриваємо новий порт, якщо індекс валідний
      if (port_to_open_idx != -1) {
        if (RS232_OpenComport(port_to_open_idx, 115200, mode, 0) == 0) {
          oscData->comport_number = port_to_open_idx;
          printf("Відкрито COM порт: %s (індекс %d)\n", oscData->com_port_name_input, oscData->comport_number);
        } else {
          printf("Не вдалося відкрити COM порт: %s\n", oscData->com_port_name_input);
        }
      }
    }

    // Кнопка "Disconnect" для закриття COM-порту
    if (GuiButton((Rectangle){panel_x + 130, current_y, 160, 30}, "Disconnect")) {
      if (oscData->comport_number != -1) {
        RS232_CloseComport(oscData->comport_number);
        oscData->comport_number = -1;
        printf("COM порт відключено.\n");
      }
    }
    current_y += 40;

    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    // Налаштування параметрів каналу A (жовтий колір)
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xFFFF00FF);
    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Channel A Scale (V/div)");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%.2f", oscData->scale_y_a), &oscData->scale_y_a, 0.1f, 1.0f);
    current_y += Vertical_Space;

    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Channel A Offset");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%d", (int)oscData->offset_y_a), &oscData->offset_y_a, -screenHeight, screenHeight * 2);
    current_y += Vertical_Space;

    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Trigger Level A");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%.2f", oscData->trigger_level_a), &oscData->trigger_level_a, 0.0f, 1.0f);
    current_y += Vertical_Space;

    GuiCheckBox((Rectangle){panel_x, current_y, 20, 20}, "Trigger Active A", &oscData->trigger_active_a);
    current_y += 40;

    // Налаштування параметрів каналу B (зелений колір)
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x00FF00FF);
    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Channel B Scale (V/div)");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%.2f", oscData->scale_y_b), &oscData->scale_y_b, 0.1f, 1.0f);
    current_y += Vertical_Space;

    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Channel B Offset");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%d", (int)oscData->offset_y_b), &oscData->offset_y_b, -screenHeight, screenHeight * 2);
    current_y += Vertical_Space;

    GuiLabel((Rectangle){panel_x, current_y, W_Controls, H_Controls}, "Trigger Level B");
    current_y += Vertical_Space;
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%.2f", oscData->trigger_level_b), &oscData->trigger_level_b, 0.0f, 1.0f);
    current_y += Vertical_Space;

    GuiCheckBox((Rectangle){panel_x, current_y, 20, 20}, "Trigger Active B", &oscData->trigger_active_b);
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

