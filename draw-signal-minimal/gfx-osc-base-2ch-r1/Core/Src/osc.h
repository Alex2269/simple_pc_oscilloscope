#ifndef __OSC_H
#define __OSC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "graphics.h"

#define WIDTH 800
#define HEIGHT 480

#define CELL_SIZE 40
#define BEGIN_RAY (CELL_SIZE * 4)
#define WIDTH_RAY (WIDTH - (CELL_SIZE-1))
#define HALF_RAY ((WIDTH_RAY - BEGIN_RAY) / 2)
#define MIN_Y_BORDER CELL_SIZE
#define MAX_Y_BORDER (HEIGHT - CELL_SIZE)
#define Y_WORKSPACE (MAX_Y_BORDER - MIN_Y_BORDER)
#define X_WORKSPACE (WIDTH - (BEGIN_RAY + CELL_SIZE))

extern uint16_t BACK_COLOR;
extern uint16_t POINT_COLOR;



/* Фільтрація сигналу для каналу A */
uint16_t filter_signal_a(uint16_t input_signal);

/* Фільтрація сигналу для каналу B */
uint16_t filter_signal_b(uint16_t input_signal);

/* Зчитування надісланих даних з мікроконтролера через USB */
void read_usb_device(uint16_t i);
/* Функція для ініціалізації осцилографа та налаштування вікна */
void oscil_init(void);

/* Основний цикл осцилографа */
void oscil_run(void);

/* синхронізація */
void process_trigger_channel(uint16_t *buffer, bool *channel_active, uint16_t trig);

/* Логіка включення тригера */
void trigger_on(uint16_t trig);

/* Режим з буферизацією */
void run_buffering(void);

/* Режим без буферизації */
void run_no_buffering(void);

/* Допоміжна функція для обробки одного каналу */
void process_channel_data(uint16_t i, uint16_t *buffer_save, float *phase, int16_t scroll_y, uint16_t adc_value);

/* Заповнення буферів результатами ADC */
void write_buffers(uint16_t i);

/* Логіка вимкнення тригера */
void fill_buffer(void);

/* Функція для очищення буфера каналу */
void clean_buffer(uint16_t *ptr_clean);

/* Поворот буфера для прокручування графіків */
void rotate_buff(uint16_t r);

/* Малювання старого графіка для каналу A */
void graph_ray_clean_a(uint16_t i);

/* Малювання старого графіка для каналу B */
void graph_ray_clean_b(uint16_t i);

/* Малювання нового графіка для каналу A */
void graph_ray_draw_a(uint16_t i);

/* Малювання нового графіка для каналу B */
void graph_ray_draw_b(uint16_t i);

/* Малювання нового графіка для вибраного каналу */
void signal_draw(uint16_t *ptr_save, int16_t scroll_x, uint16_t i, uint32_t color);

/* Малювання сітки точками по вертикалі */
void draw_vertical_grid(uint16_t i);

/* Малювання сітки точками по горизонталі */
void draw_horisontal_grid(uint16_t i);


#ifdef __cplusplus
}
#endif

#endif /* __OSC_H */
