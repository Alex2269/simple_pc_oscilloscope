#include "raygui.h"
#include "draw_signal.h"
#include <stddef.h>

// Малює сигнал одного каналу
void draw_channel_signal(float *history, int history_index, int trigger_index,
                         float offset_y, float scale_y, Color color,
                         float osc_width, float lineThickness, int history_size,
                         int points_left, int points_right, float trigger_x_pos, bool reverse)
{
    float x_step = osc_width / (points_left + points_right);

    if (!reverse) {
        for (int i = 0; i < points_left - 1; i++) {
            int idx1 = (history_index + trigger_index - points_left + i + history_size) % history_size;
            int idx2 = (history_index + trigger_index - points_left + i + 1 + history_size) % history_size;

            Vector2 p1 = { trigger_x_pos - (points_left - 1 - i) * x_step,
                           offset_y - history[idx1] * scale_y };
            Vector2 p2 = { trigger_x_pos - (points_left - 2 - i) * x_step,
                           offset_y - history[idx2] * scale_y };

            DrawLineEx(p1, p2, lineThickness, color);
        }

        for (int i = 0; i < points_right - 1; i++) {
            int idx1 = (history_index + trigger_index + i) % history_size;
            int idx2 = (history_index + trigger_index + i + 1) % history_size;

            Vector2 p1 = { trigger_x_pos + i * x_step,
                           offset_y - history[idx1] * scale_y };
            Vector2 p2 = { trigger_x_pos + (i + 1) * x_step,
                           offset_y - history[idx2] * scale_y };

            DrawLineEx(p1, p2, lineThickness, color);
        }
    } else {
        // Реверс (якщо потрібно)
        for (int i = 0; i < points_left - 1; i++) {
            int idx1 = (history_index + trigger_index - points_left + i + history_size) % history_size;
            int idx2 = (history_index + trigger_index - points_left + i + 1 + history_size) % history_size;

            Vector2 p1 = { trigger_x_pos + (points_left - i) * x_step,
                           offset_y - history[idx1] * scale_y };
            Vector2 p2 = { trigger_x_pos + (points_left - 1 - i) * x_step,
                           offset_y - history[idx2] * scale_y };

            DrawLineEx(p1, p2, lineThickness, color);
        }

        for (int i = 0; i < points_right - 1; i++) {
            int idx1 = (history_index + trigger_index + i) % history_size;
            int idx2 = (history_index + trigger_index + i + 1) % history_size;

            Vector2 p1 = { trigger_x_pos - i * x_step,
                           offset_y - history[idx1] * scale_y };
            Vector2 p2 = { trigger_x_pos - (i + 1) * x_step,
                           offset_y - history[idx2] * scale_y };

            DrawLineEx(p1, p2, lineThickness, color);
        }
    }
}

// Малює всі активні канали
void draw_signal(OscData *oscData, float osc_width, float lineThickness)
{
    const int points_total = 500;
    int points_left = (int)(oscData->trigger_offset_x / osc_width * points_total);
    int points_right = points_total - points_left;

    Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, BLUE };

    for (int i = 0; i < MAX_CHANNELS; i++) {
        ChannelSettings *ch = &oscData->channels[i];
        if (ch->active && ch->channel_history != NULL) {
            float trigger_x_pos = oscData->trigger_offset_x;
            draw_channel_signal(
                ch->channel_history,
                oscData->history_index,
                ch->trigger_index,
                ch->offset_y,
                ch->scale_y,
                channel_colors[i],
                osc_width,
                lineThickness,
                points_total,
                points_left,
                points_right,
                trigger_x_pos,
                oscData->reverse_signal
            );
        }
    }
}
