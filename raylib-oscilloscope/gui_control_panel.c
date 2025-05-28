// gui_control_panel.c

#include "raygui.h"
#include "gui_control_panel.h"
#include <stddef.h>

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
    GuiSliderBar((Rectangle){panel_x, current_y, W_Controls, H_Controls}, NULL, TextFormat("%.1f", oscData->refresh_rate_ms), &oscData->refresh_rate_ms, 10.0f, 50.0f);
    current_y += Vertical_Space;

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

