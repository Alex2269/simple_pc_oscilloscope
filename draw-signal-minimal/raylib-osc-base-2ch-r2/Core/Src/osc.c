#include "raylib.h"
#include "stdint.h"
#include "osc.h"

#include "draw_grid.h"
#include "draw_lines.h"
#include "rs232.h"
#include "find_usb_device.h"
#include "test_signal.h"

// Константи для кращої читабельності
#define ADC_RESOLUTION 4096     // Роздільна здатність АЦП (Analog-to-Digital Converter)
#define BUFFER_NOISE_REDUCTION 1.0f    // Параметр для зменшення шумів в буфері
#define TEST_SIGNAL_MULTIPLIER 1.0f    // Множник для тестового сигналу
#define TEST_SIGNAL_OFFSET 0.0f // Зсув для тестового сигналу
#define TRIGGER_MIN 100         // Мінімальне значення для тригера
#define TRIGGER_MAX 4000        // Максимальне значення для тригера

// Макрос для обмеження значення в межах
#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

uint16_t buffering = 0;         // Дані записуємо в буфер (1), (0) не записуємо
uint32_t ray_speed = 1;         // Затримка промальовування графіку

bool oscil_stop = 0;            // Статус осцилографа (зупинений чи ні)

bool channel_a = 1;             // Активність каналу A (1 - активний, 0 - неактивний)
bool channel_b = 1;             // Активність каналу B (1 - активний, 0 - неактивний)

uint32_t adc_tmp_a = 0;         // Тимчасове значення для каналу A
uint32_t adc_tmp_b = 0;         // Тимчасове значення для каналу B

uint16_t trigger = 0;           // Поріг спрацьовування тригера
uint16_t buff_scroll_x = 0;     // Зміщення буфера по осі X

int16_t scroll_x_a = 0, scroll_x_b = 0; // Зміщення буферів по осі X для каналів A і B
int16_t scroll_y_a = 0, scroll_y_b = 0; // Зміщення буферів по осі Y для каналів A і B

uint16_t buff_trigger = 0;      // Запис тригера в буфер
uint16_t fps_count = 0;         // Лічильник кадрів

uint16_t buffer_save_a[WIDTH] = { 0 };  // Буфер для каналу A
uint16_t buffer_save_b[WIDTH] = { 0 };  // Буфер для каналу B

uint16_t buffer_clean_a[WIDTH] = { 0 }; // Буфер для очищення графіка каналу A
uint16_t buffer_clean_b[WIDTH] = { 0 }; // Буфер для очищення графіка каналу B

uint8_t Comport_number = 0;     // номер комунікаційного порту

/* Фільтрація сигналу для каналу A */
uint16_t filter_signal_a(uint16_t input_signal)
{
  static uint16_t prev_output_a = 0;    // Для збереження попереднього значення
  const float alpha = 0.25f;    // коефіцієнт фільтрації (налаштуйте залежно від вимог)
  uint16_t output_signal = (uint16_t) (alpha * input_signal + (1 - alpha) * prev_output_a);

  prev_output_a = output_signal;        // зберігаємо поточне значення для наступного виклику

  return output_signal;
}

/* Фільтрація сигналу для каналу B */
uint16_t filter_signal_b(uint16_t input_signal)
{
  static uint16_t prev_output_b = 0;    // Для збереження попереднього значення
  const float alpha = 0.25f;    // коефіцієнт фільтрації (налаштуйте залежно від вимог)
  uint16_t output_signal = (uint16_t) (alpha * input_signal + (1 - alpha) * prev_output_b);

  prev_output_b = output_signal;        // зберігаємо поточне значення для наступного виклику

  return output_signal;
}

/* Зчитування надісланих даних з мікроконтролера через USB */
void read_usb_device(uint16_t i)
{
  static char str[20];          // Буфер для отриманих даних з пристрою
  static char buffer_tmp_a[5], buffer_tmp_b[5]; // Буфери для зберігання значень каналів A і B

  RS232_PollComport(Comport_number, str, 20);   // Читання даних з COM порту

  // Витягнення значень для каналів A і B з отриманих даних
  strncpy(buffer_tmp_a, str + 4, 4);    // Копіюємо значення для каналу A
  buffer_tmp_a[4] = '\0';       // Завершуємо рядок
  strncpy(buffer_tmp_b, str + 14, 4);   // Копіюємо значення для каналу B
  buffer_tmp_b[4] = '\0';       // Завершуємо рядок

  // Перетворюємо значення з рядка в цілі числа для каналів A і B
  adc_tmp_a = atoi(buffer_tmp_a);
  adc_tmp_b = atoi(buffer_tmp_b);

  adc_tmp_a = filter_signal_a(adc_tmp_a);
  adc_tmp_b = filter_signal_b(adc_tmp_b);

  /* перевірка отриманих даних */
  // printf(" adc_a %d\t adc_b %d\n", adc_tmp_a, adc_tmp_b);
}

// ======== test signal ==========
// тестовий синусоїдальний сигнал з розмахом амплітуди 1.0f
// та синусовою таблицею розміром WIDTH_RAY
// extern float phaseA[WIDTH_RAY], phaseB[WIDTH_RAY], phaseC[WIDTH_RAY];
// ============================

/* Функція для ініціалізації осцилографа та налаштування вікна */
void oscil_init(void)
{
  extern float phaseA[], phaseB[], phaseC[];
  Generate_Sine_Waves(); // Генерація тестових синусоїдальних хвиль

  // buttons_init();

  // Open communication port and other initializations
  Comport_number = find_usb_device();

  InitWindow(WIDTH, HEIGHT, "Oscilloscope");
  SetTargetFPS(60);

  // Main draw loop
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    oscil_run();                // Запуск основної функції осцилографа

    EndDrawing();
  }
  CloseWindow();
}

/* Основний цикл осцилографа */
void oscil_run(void)
{
  // Якщо увімкнена буферизація, викликаємо відповідну функцію
  if (buffering) {
    run_buffering();            // Запуск буферизації
  } else {
    run_no_buffering();         // Режим без буферизації
  }
}

/* Логіка включення тригера */
void trigger_on(uint16_t trig)
{
  uint16_t a1, a2;
  uint16_t b1, b2;
  uint16_t count = 0;

  // Для каналу A
  if (channel_a) {
    write_buffers(0);           // Очищаємо буфер
    a1 = buffer_save_a[0];
    write_buffers(0);           // Очищаємо буфер
    a2 = buffer_save_a[0];

    // Перевірка на спрацьовування тригера
    while (!((a1 < trig) && (a2 >= trig))) {
      count++;
      a1 = a2;
      write_buffers(0);         // Очищаємо буфер
      a2 = buffer_save_a[0];

      if (count > 800)
        break;                  // Якщо більше 800 спроб, припиняємо
    }

    // Малюємо дані після спрацьовування тригера
    for (uint16_t i = BEGIN_RAY; i < WIDTH_RAY; i++) {
      write_buffers(i + buff_scroll_x);
    }
  }

  // Для каналу B
  if (channel_b) {
    write_buffers(0);           // Очищаємо буфер
    b1 = buffer_save_b[0];
    write_buffers(0);           // Очищаємо буфер
    b2 = buffer_save_b[0];

    // Перевірка на спрацьовування тригера
    while (!((b1 < trig) && (b2 >= trig))) {
      count++;
      b1 = b2;
      write_buffers(0);         // Очищаємо буфер
      b2 = buffer_save_b[0];

      if (count > 800)
        break;                  // Якщо більше 800 спроб, припиняємо
    }

    // Малюємо дані після спрацьовування тригера
    for (uint16_t i = BEGIN_RAY; i < WIDTH_RAY; i++) {
      write_buffers(i + buff_scroll_x);
    }
  }
}

/* Логіка вимкнення тригера */
void trigger_off(void)
{
  // Заповнюємо буфери результатами ADC
  for (uint16_t i = BEGIN_RAY; i < WIDTH_RAY; i++) {
    write_buffers(i);
  }
}

/* Режим з буферизацією */
void run_buffering(void)
{
  if (oscil_stop)
    return;                     // Якщо осцилограф зупинений, не виконувати подальші дії

  // Якщо тригер в межах встановленого діапазону
  if (trigger > 500 && trigger < 2800) {
    trigger_on(trigger);        // Включаємо тригер
  } else {
    trigger_off();              // Вимикаємо тригер
  }

  // Малювання сітки
  draw_grid(WIDTH, HEIGHT, 50, 49);

  // Відображення графіків з каналів
  for (uint16_t i = BEGIN_RAY; i < WIDTH_RAY; i++) {
    /*
    graph_ray_clean_a(i);       // Очищаємо графік для каналу A
    graph_ray_clean_b(i);       // Очищаємо графік для каналу B

    graph_ray_draw_a(i);        // Малюємо графік для каналу A
    graph_ray_draw_b(i);        // Малюємо графік для каналу B
    */

    if (channel_a)
      signal_draw(buffer_save_a, 0, i, YELLOW);
    if (channel_b)
      signal_draw(buffer_save_b, 0, i, GREEN);

    // Відстежуємо кількість кадрів
    if (i == HALF_RAY) {
      fps_count++;
    }

    usleep(ray_speed);          // Затримка між кадрами
  }
}

/* Режим без буферизації */
void run_no_buffering(void)
{
  if (oscil_stop)
    return;                     // Якщо осцилограф зупинений, не виконувати подальші дії

  // Переміщаємо буфер
  rotate_buff(buff_scroll_x);

  // Малювання сітки
  draw_grid(WIDTH, HEIGHT, 50, 49);

  // Відображення графіків без буферизації
  for (uint16_t i = BEGIN_RAY; i < WIDTH_RAY; i++) {
    write_buffers(i);           // Заповнення буферу результатами ADC

    /*
    graph_ray_clean_a(i);       // Очищаємо графік для каналу A
    graph_ray_clean_b(i);       // Очищаємо графік для каналу B

    graph_ray_draw_a(i - 2);    // Малюємо графік для каналу A
    graph_ray_draw_b(i - 2);    // Малюємо графік для каналу B
    */

    if (channel_a)
      signal_draw(buffer_save_a, 0, i-2, YELLOW);
    if (channel_b)
      signal_draw(buffer_save_b, 0, i-2, GREEN);

    // Відстежуємо кількість кадрів
    if (i == HALF_RAY) {
      fps_count++;
    }

    usleep(ray_speed);          // Затримка між кадрами
  }
}

/* Допоміжна функція для обробки одного каналу */
void process_channel_data(uint16_t i, uint16_t *buffer_save, float *phase, int16_t scroll_y, uint16_t adc_value)
{
  buffer_save[i] = HEIGHT - (adc_value / (float)(ADC_RESOLUTION / Y_WORKSPACE)) + scroll_y;     // Масштабуємо для каналу
  buffer_save[i] *= BUFFER_NOISE_REDUCTION; // зменшуєм шуми для тесту
  buffer_save[i] += phase[i] * TEST_SIGNAL_MULTIPLIER + TEST_SIGNAL_OFFSET; // додаємо тестовий сигнал
  // Перевірка коректності меж для значень
  // buffer_save[i] = CLAMP(buffer_save[i], MIN_Y_BORDER, MAX_Y_BORDER); // Обмежуємо межі
  buffer_save[i] =
      (buffer_save[i] <
       MIN_Y_BORDER) ? MIN_Y_BORDER : ((buffer_save[i] > MAX_Y_BORDER) ? MAX_Y_BORDER : buffer_save[i]);
}

/* Заповнення буферів результатами ADC */
void write_buffers(uint16_t i)
{
  static uint16_t index;
  index = i - BEGIN_RAY;

  // if (index >= WIDTH_RAY + BEGIN_RAY) index -= WIDTH_RAY;
  index = i % WIDTH_RAY;        // Інкрементуйте індекс в межах WIDTH_RAY

  read_usb_device(index);       // Читання даних з USB пристрою
  // Оновлюємо буфери для кожного каналу окремо

  // Оновлюємо буфери для кожного каналу окремо
  if (channel_a) {
    process_channel_data(index, buffer_save_a, phaseA, scroll_y_a, adc_tmp_a);  // Канал A
  }

  if (channel_b) {
    process_channel_data(index, buffer_save_b, phaseB, scroll_y_b, adc_tmp_b);  // Канал B
  }
}

/* Поворот буфера для прокручування графіків */
void rotate_buff(uint16_t r)
{
  uint16_t tmp_a, tmp_b;
  // Поворот буфера для каналу A
  for (uint16_t i = 0; i < r; i++) {
    tmp_a = buffer_save_a[WIDTH_RAY - 1];

    for (uint16_t i = WIDTH_RAY - 1; i; i--) {
      buffer_save_a[i] = buffer_save_a[i - 1];
    }
    buffer_save_a[0] = tmp_a;

    // Якщо канал B активний, то робимо те ж саме для каналу B
    if (channel_b) {
      tmp_b = buffer_save_b[WIDTH_RAY - 1];
      for (uint16_t i = WIDTH_RAY - 1; i; i--) {
        buffer_save_b[i] = buffer_save_b[i - 1];
      }
      buffer_save_b[0] = tmp_b;
    }
  }
}

/*
// Якщо потрібно переміщувати буфер, можна використовувати більш ефективні методи для роботи з пам'яттю.
void rotate_buff(uint16_t r)
{
  // Використовуємо більш ефективний зсув буферу, за допомогою memmove (якщо доступно)
  if (r > 0)
  {
    memmove(buffer_save_a + r, buffer_save_a, (WIDTH_RAY - r) * sizeof(uint16_t));
    memset(buffer_save_a, 0, r * sizeof(uint16_t));     // Заповнення нулями
}
} */

/* Малювання старого графіка для каналу A */
void graph_ray_clean_a(uint16_t i)
{
  if (i >= WIDTH_RAY)
    i -= WIDTH_RAY;

  display_signal(i, buffer_clean_a[i], i + 1, buffer_clean_a[i + 1], BLACK);
}

/* Малювання старого графіка для каналу B */
void graph_ray_clean_b(uint16_t i)
{
  if (i >= WIDTH_RAY)
    i -= WIDTH_RAY;

  display_signal(i, buffer_clean_b[i], i + 1, buffer_clean_b[i + 1], BLACK);
}

/* Малювання нового графіка для каналу A */
void graph_ray_draw_a(uint16_t i)
{
  if (i >= WIDTH_RAY)
    i -= WIDTH_RAY;

  // Оновлюємо лише необхідні пікселі
  if (buffer_save_a[i] != buffer_clean_a[i]) {
    // не малюємо останню лінію. (позбуваємось смітт в кінці екрану)
    if (i <= WIDTH_RAY - 4)
      display_signal(i, buffer_save_a[i], i + 1, buffer_save_a[i + 1], YELLOW);

    buffer_clean_a[i] = buffer_save_a[i];
  }
}

/* Малювання нового графіка для каналу B */
void graph_ray_draw_b(uint16_t i)
{
  if (i >= WIDTH_RAY)
    i -= WIDTH_RAY;

  // Оновлюємо лише необхідні пікселі
  if (buffer_save_b[i] != buffer_clean_b[i]) {
    // не малюємо останню лінію. (позбуваємось сміття в кінці екрану)
    if (i <= WIDTH_RAY - 4)
      display_signal(i, buffer_save_b[i], i + 1, buffer_save_b[i + 1], GREEN);

    buffer_clean_b[i] = buffer_save_b[i];
  }
}

/* Малювання нового графіка для вибраного каналу */
void signal_draw(uint16_t *ptr_save, int16_t scroll_x, uint16_t i, Color color)
{
  int index = /*scroll_x + */ i;
  if (index > WIDTH_RAY)
    index -= WIDTH_RAY;

  // не малюємо останню лінію. (позбуваємось сміття в кінці екрану)
  if (index > (BEGIN_RAY + 2) && index < (WIDTH_RAY - 2))       // не малюємо початок і кінець
    display_signal(i, *(ptr_save + index), i + 1, *(ptr_save + index + 1), color);
}

