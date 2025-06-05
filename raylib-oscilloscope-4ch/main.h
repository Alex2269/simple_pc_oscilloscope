// main.h

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_CHANNELS 4

typedef struct {
    bool active;
    float scale_y;               // Масштабування по вертикалі
    float offset_y;              // Зміщення по вертикалі
    float trigger_level;         // Рівень тригера для (0..1)
    bool trigger_active;         // Чи активний тригер
    float *channel_history; // Вказівник на буфер історії (масив float довжиною 500)

    int trigger_index;           // Індекс точки тригера в історії
    float trigger_index_smooth;  // Плавне значення індексу тригера (float)
    bool trigger_locked;         // Прапорець блокування оновлення тригера
    int frames_since_trigger;    // Лічильник кадрів після спрацювання тригера
} ChannelSettings;


// Структура для зберігання стану осцилографа і параметрів відображення
typedef struct OscData {
    ChannelSettings channels[MAX_CHANNELS];
    int active_channel;           // індекс активного каналу
    int comport_number;           // Індекс відкритого COM-порту (-1 якщо не відкрито)
    int ray_speed;                // Затримка читання даних у мікросекундах
    int adc_tmp_a;                // Поточне відфільтроване значення ADC каналу A
    int adc_tmp_b;                // Поточне відфільтроване значення ADC каналу B
    int history_index;            // Поточний індекс запису в історії (циклічний буфер)
    float refresh_rate_ms;        // Частота оновлення інтерфейсу (мс)
    bool auto_connect;            // Прапорець автоматичного підключення до COM-порту
    char com_port_name_input[20]; // Ім'я COM-порту, введене користувачем
    bool com_port_name_edit_mode; // Режим редагування імені COM-порту

    // Поля для плавного оновлення тригера і блокування оновлення
    float trigger_offset_x;       // Горизонтальне зміщення позиції тригера (пікселі)
    bool reverse_signal;          // Прапорець реверсу напрямку малювання сигналу
} OscData;


#endif // MAIN_H

