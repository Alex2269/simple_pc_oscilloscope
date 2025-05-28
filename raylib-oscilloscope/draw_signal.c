// file draw_signal.c

#include "raygui.h"
#include "draw_signal.h"
#include <stddef.h>

// Функція малює сигнал одного каналу осцилографа на екрані
// history - масив з історією значень сигналу
// history_index - поточний індекс у буфері історії
// trigger_index - індекс точки тригера в історії
// offset_y - вертикальне зміщення сигналу на екрані
// scale_y - масштабування по вертикалі (амплітуда)
// color - колір лінії сигналу
// osc_width - ширина області відображення сигналу по горизонталі
// lineThickness - товщина лінії сигналу
// history_size - розмір буфера історії сигналу
// points_left - кількість точок ліворуч від тригера для відображення
// points_right - кількість точок праворуч від тригера для відображення
// trigger_x_pos - горизонтальна позиція тригера на екрані (пікселі)
// reverse - прапорець реверсу сигналу (малювання у зворотному напрямку)
void draw_channel_signal(float *history, int history_index, int trigger_index,
                         float offset_y, float scale_y, Color color,
                         float osc_width, float lineThickness, int history_size,
                         int points_left, int points_right, float trigger_x_pos, bool reverse)
{
    // Крок по горизонталі між точками сигналу
    float x_step = osc_width / (points_left + points_right);

    if (!reverse) { // Малюємо сигнал у звичайному напрямку (зліва направо)

        // Малюємо ліву частину сигналу (до тригера)
        for (int i = 0; i < points_left - 1; i++) {
            // Обчислюємо індекси точок у буфері історії з урахуванням циклічності
            int idx1 = (history_index + trigger_index - points_left + i + history_size) % history_size;
            int idx2 = (history_index + trigger_index - points_left + i + 1 + history_size) % history_size;

            // Обчислюємо координати двох точок для лінії, зсув по X і масштаб по Y
            Vector2 p1 = { trigger_x_pos - (points_left - 1 - i) * x_step,
                        offset_y - history[idx1] * scale_y };
            Vector2 p2 = { trigger_x_pos - (points_left - 2 - i) * x_step,
                        offset_y - history[idx2] * scale_y };

            // Малюємо відрізок лінії між двома точками
            DrawLineEx(p1, p2, lineThickness, color);
        }

        // Малюємо праву частину сигналу (після тригера)
        for (int i = 0; i < points_right - 1; i++) {
            // Індекси точок у буфері для правої частини
            int idx1 = (history_index + trigger_index + i) % history_size;
            int idx2 = (history_index + trigger_index + i + 1) % history_size;

            // Координати точок для відрізка лінії
            Vector2 p1 = { trigger_x_pos + i * x_step,
                        offset_y - history[idx1] * scale_y };
            Vector2 p2 = { trigger_x_pos + (i + 1) * x_step,
                        offset_y - history[idx2] * scale_y };

            // Малюємо лінію між точками
            DrawLineEx(p1, p2, lineThickness, color);
        }
    } else {
        // Режим реверсу: малюємо сигнал у зворотному напрямку,
        // але центруємо його на позиції тригера trigger_x_pos

        // Малюємо ліву частину сигналу (до тригера) у реверсі
        for (int i = 0; i < points_left - 1; i++) {
            int idx1 = (history_index + trigger_index - points_left + i + history_size) % history_size;
            int idx2 = (history_index + trigger_index - points_left + i + 1 + history_size) % history_size;

            // Координати точок з інвертованим напрямком по X
            Vector2 p1 = { trigger_x_pos + (points_left - i) * x_step,
                           offset_y - history[idx1] * scale_y };
            Vector2 p2 = { trigger_x_pos + (points_left - 1 - i) * x_step,
                           offset_y - history[idx2] * scale_y };

            DrawLineEx(p1, p2, lineThickness, color);
        }

        // Малюємо праву частину сигналу (після тригера) у реверсі
        for (int i = 0; i < points_right - 1; i++) {
            int idx1 = (history_index + trigger_index + i) % history_size;
            int idx2 = (history_index + trigger_index + i + 1) % history_size;

            // Координати точок з інвертованим напрямком по X
            Vector2 p1 = { trigger_x_pos - i * x_step,
                           offset_y - history[idx1] * scale_y };
            Vector2 p2 = { trigger_x_pos - (i + 1) * x_step,
                           offset_y - history[idx2] * scale_y };

            DrawLineEx(p1, p2, lineThickness, color);
        }
    }
}

// Функція малює сигнал на екрані осцилографа
// oscData - структура з даними сигналу та параметрами відображення
// osc_width - ширина області відображення сигналу
// lineThickness - товщина лінії сигналу
void draw_signal(OscData *oscData, float osc_width, float lineThickness)
{
    // Загальна кількість точок в історії сигналу
    const int points_total = 500;
    const int history_size = 500;

    // Якщо увімкнено режим реверсу сигналу
    if (oscData->reverse_signal) {
        int points_to_draw = points_total; // Малюємо весь буфер сигналу

        // Нормалізоване положення тригера відносно ширини осцилографа
        float trigger_pos_normalized = oscData->trigger_offset_x / osc_width;

        // Зсув у точках відносно загальної кількості точок
        int offset_points = (int)(trigger_pos_normalized * points_total);

        // Обчислення початкових індексів для малювання сигналів каналів з урахуванням реверсу
        int start_index_a = (oscData->trigger_index_a + offset_points) % points_total;
        int start_index_b = (oscData->trigger_index_b + offset_points) % points_total;

        // Малюємо сигнал каналу А, починаючи з обчисленого індексу
        draw_channel_signal(oscData->channel_a_history, oscData->history_index, start_index_a,
                            oscData->offset_y_a, oscData->scale_y_a, YELLOW,
                            osc_width, lineThickness, points_total,
                            points_to_draw, 0,  // Малюємо всі точки вправо від початку
                            0.0f, oscData->reverse_signal);

        // Малюємо сигнал каналу B, починаючи з обчисленого індексу
        draw_channel_signal(oscData->channel_b_history, oscData->history_index, start_index_b,
                            oscData->offset_y_b, oscData->scale_y_b, GREEN,
                            osc_width, lineThickness, points_total,
                            points_to_draw, 0,
                            0.0f, oscData->reverse_signal);
    }

    // Обчислюємо кількість точок ліворуч від тригера
    int points_left = (int)(oscData->trigger_offset_x / osc_width * points_total);
    // Кількість точок праворуч від тригера
    int points_right = points_total - points_left;

    // Малюємо сигнал каналу А з позиції тригера, з урахуванням точок ліворуч і праворуч
    draw_channel_signal(oscData->channel_a_history, oscData->history_index, oscData->trigger_index_a,
                        oscData->offset_y_a, oscData->scale_y_a, YELLOW,
                        osc_width, lineThickness, history_size, points_left, points_right,
                        oscData->trigger_offset_x, oscData->reverse_signal);

    // Малюємо сигнал каналу B з позиції тригера, з урахуванням точок ліворуч і праворуч
    draw_channel_signal(oscData->channel_b_history, oscData->history_index, oscData->trigger_index_b,
                        oscData->offset_y_b, oscData->scale_y_b, GREEN,
                        osc_width, lineThickness, history_size, points_left, points_right,
                        oscData->trigger_offset_x, oscData->reverse_signal);
}


