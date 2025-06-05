#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // usleep
#include <stdbool.h>
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

#define HISTORY_SIZE 500
#define WORKSPACE_HEIGHT 550

int fontSize = 24;
int LineSpacing = 0;
Font font;

// Пошук індексу тригера (front-edge trigger)
int find_trigger_index_px(float *history, int history_index, float trigger_level_px, int history_size) {
    for (int i = 0; i < history_size - 1; i++) {
        int idx0 = (history_index + i) % history_size;
        int idx1 = (history_index + i + 1) % history_size;
        float v0 = history[idx0];
        float v1 = history[idx1];
        if (v0 < trigger_level_px && v1 >= trigger_level_px) {
            return i;
        }
    }
    return 0;
}

void read_usb_device(OscData *data) {
    static char str[256];

    if (data->comport_number < 0) {
        data->adc_tmp_a = 0;
        data->adc_tmp_b = 0;
        return;
    }

    int bytes_read = RS232_PollComport(data->comport_number, (unsigned char*)str, sizeof(str) - 1);
    if (bytes_read <= 0) return;
    str[bytes_read] = '\0';

    usleep(data->ray_speed);

    parse_data(str, &data->adc_tmp_a, &data->adc_tmp_b);

    float scaled_a = ((float)data->adc_tmp_a / 4095) * WORKSPACE_HEIGHT;
    float scaled_b = ((float)data->adc_tmp_b / 4095) * WORKSPACE_HEIGHT;

    if (data->channels[0].channel_history != NULL)
        data->channels[0].channel_history[data->history_index] = scaled_a;
    if (data->channels[1].channel_history != NULL)
        data->channels[1].channel_history[data->history_index] = scaled_b;

    data->history_index = (data->history_index + 1) % HISTORY_SIZE;
}

void init_osc_data(OscData *oscData) {
    static float channel_a_history[HISTORY_SIZE] = {0};
    static float channel_b_history[HISTORY_SIZE] = {0};

    oscData->comport_number = -1;
    oscData->active_channel = 0;
    oscData->refresh_rate_ms = 20.0f;
    oscData->auto_connect = false;
    oscData->com_port_name_edit_mode = false;
    strcpy(oscData->com_port_name_input, "COM1");
    oscData->ray_speed = 1000;

    for (int i = 0; i < MAX_CHANNELS; i++) {
        oscData->channels[i].scale_y = 0.5f;
        oscData->channels[i].offset_y = 0;
        oscData->channels[i].trigger_level = 0.5f;
        oscData->channels[i].trigger_active = false;
        oscData->channels[i].active = (i < 2);
        oscData->channels[i].channel_history = NULL;

        oscData->channels[i].trigger_index = 0;
        oscData->channels[i].trigger_index_smooth = 0.0f;
        oscData->channels[i].trigger_locked = false;
        oscData->channels[i].frames_since_trigger = 0;
    }

    oscData->channels[0].channel_history = channel_a_history;
    oscData->channels[1].channel_history = channel_b_history;

    oscData->history_index = 0;
    oscData->trigger_offset_x = 100;
    oscData->reverse_signal = false;
}

void update_trigger_indices(OscData *oscData) {
    float smoothing_alpha = 0.1f;
    const int RESET_THRESHOLD = 5; // Кількість кадрів для скидання тригера (~0.08 сек при 60 FPS)

    for (int i = 0; i < MAX_CHANNELS; i++) {
        ChannelSettings *ch = &oscData->channels[i];
        if (ch->active && ch->trigger_active && ch->channel_history != NULL) {
            float trigger_level_px = ch->trigger_level * WORKSPACE_HEIGHT;

            int found_offset = find_trigger_index_px(
                ch->channel_history,
                oscData->history_index,
                trigger_level_px,
                HISTORY_SIZE);

            int absolute_trigger_index = (oscData->history_index + 1 + found_offset) % HISTORY_SIZE;

            if (!ch->trigger_locked) {
                // Плавне оновлення позиції тригера
                ch->trigger_index_smooth = smoothing_alpha * absolute_trigger_index + (1.0f - smoothing_alpha) * ch->trigger_index_smooth;
                ch->trigger_index = (int)(ch->trigger_index_smooth + 0.5f);

                // Перевірка стабільності позиції тригера
                if (abs(ch->trigger_index - absolute_trigger_index) < 2) {
                    ch->frames_since_trigger++;
                    if (ch->frames_since_trigger > 3) {
                        ch->trigger_locked = true;  // Тригер спрацював і заблокувався
                        ch->frames_since_trigger = 0; // Скидаємо лічильник для наступного циклу
                    }
                } else {
                    ch->frames_since_trigger = 0;
                }
            } else {
                // Тригер заблокований, рахуємо час до скидання
                ch->frames_since_trigger++;
                if (ch->frames_since_trigger > RESET_THRESHOLD) {
                    ch->trigger_locked = false;   // Розблокуємо тригер для нового спрацювання
                    ch->frames_since_trigger = 0; // Скидаємо лічильник
                }
            }
        } else {
            // Якщо канал не активний або тригер не активний - скидаємо стан
            ch->trigger_locked = false;
            ch->frames_since_trigger = 0;
            ch->trigger_index = 0;
            ch->trigger_index_smooth = 0.0f;
        }
    }
}

int main(void) {
    const int screenWidth = 1000;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Raylib Oscilloscope with Trigger and Scaling");
    font = LoadFontUnicode("bold.ttf", fontSize, LineSpacing);

    SetTargetFPS(60);

    OscData oscData = {0};
    init_osc_data(&oscData);

    // Ініціалізація стилів GUI
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

    // Автоматичне відкриття COM-порту
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

        if (frameTime * 1000.0f >= oscData.refresh_rate_ms) {
            read_usb_device(&oscData);
            update_trigger_indices(&oscData);
            frameTime = 0.0f;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        int osc_width = screenWidth - 360;
        int osc_height = screenHeight;

        DrawRectangle(0, 0, osc_width, osc_height, BLACK);
        draw_grid(osc_width, osc_height);

        Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, BLUE };
        for (int i = 0; i < MAX_CHANNELS; i++) {
            if (oscData.channels[i].active && oscData.channels[i].channel_history != NULL) {
                float last_value = oscData.channels[i].channel_history[(oscData.history_index + HISTORY_SIZE - 1) % HISTORY_SIZE];
                DrawTextEx(font, TextFormat("Ch%d: %.0f", i, last_value), (Vector2){10, 10 + i*30}, fontSize, 2, channel_colors[i]);
            }
        }

        ChannelSettings *ch = &oscData.channels[oscData.active_channel];
        if (ch->active && ch->trigger_active) {
            float trigger_level_px = ch->trigger_level * WORKSPACE_HEIGHT;
            float y_trigger = /*osc_height / 2 +*/ ch->offset_y - trigger_level_px * ch->scale_y;
            DrawLine(0, (int)y_trigger, osc_width, (int)y_trigger, channel_colors[oscData.active_channel]);
        }

        if (ch->active && ch->channel_history != NULL) {
            float x_step = (float)osc_width / HISTORY_SIZE;
            int rel_index = (HISTORY_SIZE + ch->trigger_index - oscData.history_index) % HISTORY_SIZE;
            float trigger_x_pos = x_step * rel_index;
            DrawLine((int)trigger_x_pos, 0, (int)trigger_x_pos, screenHeight, RED);
        }

        int points_total = HISTORY_SIZE;
        int points_left = (int)(oscData.trigger_offset_x / osc_width * points_total);
        int points_right = points_total - points_left;

        // generate_test_signals(&oscData, 500, 0.0f);

        for (int i = 0; i < MAX_CHANNELS; i++) {
            ChannelSettings *ch = &oscData.channels[i];
            if (ch->active && ch->channel_history != NULL) {
                draw_channel_signal(
                    ch->channel_history,
                    oscData.history_index,
                    ch->trigger_index,
                    ch->offset_y,
                    ch->scale_y,
                    channel_colors[i],
                    (float)osc_width,
                    2.0f,
                    points_total,
                    points_left,
                    points_right,
                    oscData.trigger_offset_x,
                    oscData.reverse_signal
                );
            }
        }

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

