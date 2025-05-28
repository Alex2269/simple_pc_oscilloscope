// main.h

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include "raylib.h"

// Структура для зберігання стану осцилографа і параметрів відображення
typedef struct OscData {
    int comport_number;           // Індекс відкритого COM-порту (-1 якщо не відкрито)
    int ray_speed;                // Затримка читання даних у мікросекундах
    int adc_tmp_a;                // Поточне відфільтроване значення ADC каналу A
    int adc_tmp_b;                // Поточне відфільтроване значення ADC каналу B
    float channel_a_history[500]; // Історія значень каналу A (кількість точок 500)
    float channel_b_history[500]; // Історія значень каналу B
    int history_index;            // Поточний індекс запису в історії (циклічний буфер)
    float scale_y_a;              // Масштабування по вертикалі для каналу A
    float scale_y_b;              // Масштабування по вертикалі для каналу B
    float offset_y_a;             // Зміщення по вертикалі для каналу A
    float offset_y_b;             // Зміщення по вертикалі для каналу B
    float refresh_rate_ms;        // Частота оновлення інтерфейсу (мс)
    bool auto_connect;            // Прапорець автоматичного підключення до COM-порту
    char com_port_name_input[20]; // Ім'я COM-порту, введене користувачем
    bool com_port_name_edit_mode; // Режим редагування імені COM-порту

    float trigger_level_a;        // Рівень тригера для каналу A (0..1)
    float trigger_level_b;        // Рівень тригера для каналу B (0..1)
    int trigger_index_a;          // Індекс точки тригера в історії каналу A
    int trigger_index_b;          // Індекс точки тригера в історії каналу B
    bool trigger_active_a;        // Чи активний тригер для каналу A
    bool trigger_active_b;        // Чи активний тригер для каналу B

    // Поля для плавного оновлення тригера і блокування оновлення
    float trigger_index_a_smooth;
    float trigger_index_b_smooth;
    bool trigger_locked_a;
    bool trigger_locked_b;
    int frames_since_trigger_a;
    int frames_since_trigger_b;
    float trigger_offset_x;       // Горизонтальне зміщення позиції тригера (пікселі)
    bool reverse_signal;          // Прапорець реверсу напрямку малювання сигналу
} OscData;

#endif // MAIN_H

