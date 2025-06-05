// generate_test_signals.c
#include "math.h"
#include <stddef.h>
#include "generate_test_signals.h"

    // oscData.offset_y_a = screenHeight / 4.0f * 2.5f + 200;
    // oscData.offset_y_b = screenHeight / 4.0f * 3.5f + 200;

        // generate_test_signals(&oscData, 500, 100);
        // Приклад виклику
        // draw_signal(&oscData, osc_width, 2.0f);

        /*
        float current_time = 0.0f;
        generate_test_signals(&oscData, 500, current_time);
        current_time += 0.1f; // Збільшуємо час для наступного кадру */


void generate_test_signals(OscData *data, int history_size, float time)
{
    // Перевірка наявності буферів історії для каналів 0 і 1
    if (data->channels[0].channel_history == NULL || data->channels[1].channel_history == NULL) return;

    for (int i = 0; i < history_size; i++) {
        float t = time + i * 0.1f;
        data->channels[0].channel_history[i] = 200.0f + 100.0f * sinf(t);       // Синусоїда для каналу 0 (A)
        data->channels[1].channel_history[i] = 200.0f + 100.0f * cosf(t * 0.5f); // Косинусоїда для каналу 1 (B)
    }
}
