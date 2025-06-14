#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // usleep
#include <stdbool.h>
#include "raylib.h"
#include "rs232.h"
#include "LoadFontUnicode.h"
#include "draw_grid.h"

#include "main.h"
#include "trigger.h"
#include "parse_data.h"
#include "gui_control_panel.h"
#include "draw_signal.h"
#include "generate_test_signals.h"
#include "cursor.h"

#include <stdint.h>

int fontSize = 24;
int LineSpacing = 0;
Font font;

void read_usb_device(OscData *data) {
    static uint8_t buffer[PACKET_SIZE];
    static int buf_idx = 0;

    if (data->comport_number < 0) {
        // Очистка або ініціалізація
        return;
    }

    uint8_t temp_buf[256];
    int bytes_read = RS232_PollComport(data->comport_number, temp_buf, sizeof(temp_buf));
    if (bytes_read <= 0) return;

    for (int i = 0; i < bytes_read; i++) {
        uint8_t byte = temp_buf[i];

        if (buf_idx == 0) {
            // Чекаємо стартовий байт
            if (byte == 0xAA) {
                buffer[buf_idx++] = byte;
            }
        } else {
            buffer[buf_idx++] = byte;
            if (buf_idx == PACKET_SIZE) {
                // Маємо повний пакет
                uint16_t channel_values[4];
                if (parse_binary_packet(buffer, channel_values) == 0) {
                    data->adc_tmp_a = channel_values[0];
                    data->adc_tmp_b = channel_values[1];
                    data->adc_tmp_c = channel_values[2];
                    data->adc_tmp_d = channel_values[3];

                    // Масштабування
                    float scaled_a = ((float)data->adc_tmp_a / 4095) * WORKSPACE_HEIGHT;
                    float scaled_b = ((float)data->adc_tmp_b / 4095) * WORKSPACE_HEIGHT;
                    float scaled_c = ((float)data->adc_tmp_c / 4095) * WORKSPACE_HEIGHT;
                    float scaled_d = ((float)data->adc_tmp_d / 4095) * WORKSPACE_HEIGHT;

                    if (data->channels[0].channel_history != NULL)
                        data->channels[0].channel_history[data->history_index] = scaled_a;
                    if (data->channels[1].channel_history != NULL)
                        data->channels[1].channel_history[data->history_index] = scaled_b;
                    if (data->channels[2].channel_history != NULL)
                        data->channels[2].channel_history[data->history_index] = scaled_c;
                    if (data->channels[3].channel_history != NULL)
                        data->channels[3].channel_history[data->history_index] = scaled_d;

                    data->history_index = (data->history_index + 1) % HISTORY_SIZE;
                } else {
                    // Помилка розбору пакета, можна логувати або ігнорувати
                }
                buf_idx = 0; // Готуємось до наступного пакета
            }
        }
    }
}

void init_osc_data(OscData *oscData) {
    static float channel_a_history[HISTORY_SIZE] = {0};
    static float channel_b_history[HISTORY_SIZE] = {0};
    static float channel_c_history[HISTORY_SIZE] = {0};
    static float channel_d_history[HISTORY_SIZE] = {0};

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
        oscData->channels[0].offset_y = 300;
        oscData->channels[1].offset_y = 400;
        oscData->channels[2].offset_y = 500;
        oscData->channels[3].offset_y = 600;
        oscData->channels[0].trigger_level = 0.25f;
        oscData->channels[1].trigger_level = 0.25f;
        oscData->channels[2].trigger_level = 0.25f;
        oscData->channels[3].trigger_level = 0.30f;
        oscData->channels[i].trigger_hysteresis_px = 0.25f;
        oscData->channels[i].trigger_active = false;
        oscData->channels[2].trigger_active = true;
        oscData->channels[i].active = (i < 4);
        oscData->channels[i].channel_history = NULL;

        oscData->channels[i].trigger_index = 0;
        oscData->channels[i].trigger_index_smooth = 0.0f;
        oscData->channels[i].trigger_locked = false;
        oscData->channels[i].frames_since_trigger = 0;
    }

    oscData->channels[0].channel_history = channel_a_history;
    oscData->channels[1].channel_history = channel_b_history;
    oscData->channels[2].channel_history = channel_c_history;
    oscData->channels[3].channel_history = channel_d_history;

    oscData->history_index = 0;
    oscData->trigger_offset_x = 100;
    oscData->reverse_signal = false;
    oscData->movement_signal = false;
    oscData->test_signal = true;
}

int main(void) {
    const int screenWidth = 1000;
    const int screenHeight = 600;

    int osc_width = screenWidth - 360;
    int osc_height = screenHeight;

    // Встановлюємо прапорець для мультисемплінгу (покращення якості графіки)
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(screenWidth, screenHeight, "Raylib Oscilloscope with Trigger and Scaling");
    font = LoadFontUnicode("bold.ttf", fontSize, LineSpacing);

    SetTargetFPS(60);

    Cursor cursors[2];
    cursors[0] = InitCursor(osc_width / 3.0f, DEFAULT_CURSOR_TOP_Y, DEFAULT_CURSOR_WIDTH, DEFAULT_CURSOR_HEIGHT, RED, 0, 100);
    cursors[1] = InitCursor(osc_width * 2 / 3.0f, DEFAULT_CURSOR_TOP_Y, DEFAULT_CURSOR_WIDTH, DEFAULT_CURSOR_HEIGHT, BLUE, 0, 100);

    // Ініціалізація центрального прямокутника (ручки) між курсорами
    DragRect centerRect = {
        .x = (cursors[0].x + cursors[1].x) / 2,  // центр по горизонталі
        .y = cursors[0].y,                        // початкова вертикальна позиція лінії
        .width = 20,                             // ширина прямокутника
        .height = 10,                            // висота прямокутника
        .color = LIGHTGRAY,                       // колір прямокутника
        .isDragging = false                      // спочатку не перетягується
    };

    OscData oscData = {0};
    init_osc_data(&oscData);

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

        DrawRectangle(0, 0, osc_width, osc_height, BLACK);
        draw_grid(osc_width, osc_height);

        // Малювання курсорів, ліній, ручки та тексту
        DrawCursorsAndDistance(cursors, 2, font, fontSize, &centerRect);

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

        // generate_test_signals(&oscData, 500, 0.0f);

        draw_signal(&oscData, osc_width, 2.0f);

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

