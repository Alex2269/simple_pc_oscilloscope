#include "osc.h"
// #include "draw_grid.h"
#include "rs232.h"
#include "find_usb_device.h"
#include "test_signal.h"

#define WIDTH 800
#define HEIGHT 480

#define CELL_SIZE 40
#define BEGIN_RAY (CELL_SIZE * 1)
#define WIDTH_RAY (WIDTH - CELL_SIZE+2)
#define HALF_RAY ((WIDTH_RAY - BEGIN_RAY) / 2)

#define MIN_Y_BORDER CELL_SIZE
#define MAX_Y_BORDER (HEIGHT - CELL_SIZE)

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

uint16_t buff_scroll_x = 0;     // Зміщення буфера по осі X

int16_t scroll_x_a = 0, scroll_x_b = 0; // Зміщення буферів по осі X для каналів A і B
int16_t scroll_y_a = 0, scroll_y_b = 0; // Зміщення буферів по осі Y для каналів A і B

uint16_t fps_count = 0;         // Лічильник кадрів

uint16_t buffer_save_a[WIDTH_RAY] = { 0 };  // Буфер для каналу A
uint16_t buffer_save_b[WIDTH_RAY] = { 0 };  // Буфер для каналу B

uint16_t buffer_clean_a[WIDTH_RAY] = { 0 }; // Буфер для очищення графіка каналу A
uint16_t buffer_clean_b[WIDTH_RAY] = { 0 }; // Буфер для очищення графіка каналу B

uint16_t trigger_a = 0;         // Змінна тригера
uint16_t buff_trigger = 0;      // Запис тригера в буфер

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
extern float phaseA[WIDTH_RAY], phaseB[WIDTH_RAY], phaseC[WIDTH_RAY];
// ============================

/* Функція для ініціалізації осцилографа та налаштування вікна */
void oscil_init(void)
{
  // extern float phaseA[], phaseB[], phaseC[];
  Generate_Sine_Waves(); // Генерація тестових синусоїдальних хвиль
  // buttons_init();

  // Open communication port and other initializations
  Comport_number = find_usb_device();

  // InitWindow(WIDTH, HEIGHT, "Oscilloscope");
  // SetTargetFPS(60);
  // Main draw loop
  // while (!WindowShouldClose())
  {
    // BeginDrawing();
    // ClearBackground(BLACK);
    oscil_run(); // Запуск основної функції осцилографа
    // EndDrawing();
  }
  // CloseWindow();
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

/* синхронізація */
void process_trigger_channel(uint16_t *buffer, bool *channel_active, uint16_t trig)
{
  if (*channel_active) {
    clean_buffer(buffer);       // Очищаємо буфер для визначеного каналу перед початком роботи
    uint16_t a1, a2;
    uint16_t count = 0;

    // Ініціалізація перших двох значень
    write_buffers(0);
    a1 = buffer[0];             // Перше значення
    write_buffers(1);
    a2 = buffer[1];             // Друге значення

    // Пошук фронту: шукаємо перехід через тригер
    while (!((a1 < trig - 10) && (a2 >= trig + 10)) && count < WIDTH_RAY) {
      count++;
      clean_buffer(buffer);
      a1 = a2;
      write_buffers(count);
      a2 = buffer[count];
    }

    // Якщо тригер не знайдений для активного каналу
    if (count < WIDTH_RAY) {
      fill_buffer();            // Якщо знайшли тригер
    } else {
      printf("Тригер не знайдений\n");
    }

    // printf("a1: %d, a2: %d, count: %d\n", a1, a2, count); // Перевірка значень
  }
}

/* Логіка включення тригера */
void trigger_on(uint16_t trig)
{
  // Якщо потрібно обробляти канал A
  if (channel_a)
    process_trigger_channel(buffer_save_a, &channel_a, trig);

  // Якщо потрібно обробляти канал B
  if (channel_b)
    process_trigger_channel(buffer_save_b, &channel_b, trig);
}

/* Режим з буферизацією */
void run_buffering(void)
{
  if (oscil_stop)
    return;                     // Якщо осцилограф зупинений, не виконувати подальші дії

  // Якщо тригер в межах встановленого діапазону
  if (trigger_a >= TRIGGER_MIN && trigger_a <= TRIGGER_MAX) {
    trigger_on(trigger_a);      // Включаємо тригер
  } else {
    fill_buffer();              // Вимикаємо тригер
  }

  // Відображення графіків з каналів
  for (uint16_t i = BEGIN_RAY; i < WIDTH_RAY; i++) {
    if (channel_a) {
      // graph_ray_clean_a(i); // Очищаємо графік для каналу A
      // graph_ray_draw_a(i); // Малюємо графік для каналу A
      signal_draw(buffer_save_a, scroll_x_a, i, YELLOW);        // Малюємо графік для каналу A
    }

    if (channel_b) {
      // graph_ray_clean_b(i); // Очищаємо графік для каналу B
      // graph_ray_draw_b(i); // Малюємо графік для каналу B
      signal_draw(buffer_save_b, scroll_x_b, i, GREEN); // Малюємо графік для каналу B
    }

    if (!(i % CELL_SIZE)) {
      // Оновлюємо сітку кожні CELL_SIZE пікселя
      draw_horisontal_grid(i);
      draw_vertical_grid(i);
    }

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

  // Відображення графіків без буферизації
  for (uint16_t i = BEGIN_RAY; i < WIDTH_RAY; i++) {
    write_buffers(i);           // Заповнення буферу результатами ADC
    if (channel_a) {
      // graph_ray_clean_a(i); // Очищаємо графік для каналу A
      // graph_ray_draw_a(i-2); // Малюємо графік для каналу A
      signal_draw(buffer_save_a, scroll_x_a, i - 2, YELLOW);    // Малюємо графік для каналу A
    }

    if (channel_b) {
      // graph_ray_clean_b(i); // Очищаємо графік для каналу B
      // graph_ray_draw_b(i-2); // Малюємо графік для каналу B
      signal_draw(buffer_save_b, scroll_x_b, i - 2, GREEN);     // Малюємо графік для каналу B
    }

    if (!(i % CELL_SIZE)) {
      // Оновлюємо сітку кожні CELL_SIZE пікселя
      draw_horisontal_grid(i);
      draw_vertical_grid(i);
    }

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
  buffer_save[i] *= BUFFER_NOISE_REDUCTION;     // зменшуєм шуми для тесту
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
  if (channel_a) {
    process_channel_data(index, buffer_save_a, phaseA, scroll_y_a, adc_tmp_a);  // Канал A
  }

  if (channel_b) {
    process_channel_data(index, buffer_save_b, phaseB, scroll_y_b, adc_tmp_b);  // Канал B
  }
}

/* Логіка вимкнення тригера */
void fill_buffer(void)
{
  // Заповнюємо буфери результатами ADC
  for (uint16_t i = BEGIN_RAY; i < WIDTH_RAY + BEGIN_RAY; i++) {
    write_buffers(i);
  }
}

/* Функція для очищення буфера каналу */
void clean_buffer(uint16_t *ptr_clean)
{
  // Очищаємо всі елементи буфера каналу X
  memset(ptr_clean, 0, WIDTH_RAY * sizeof(uint16_t));
}

/* Поворот буфера для прокручування графіків */
void rotate_buff(uint16_t r)
{
  // Використовуємо більш ефективний зсув буферу, за допомогою memmove (якщо доступно)
  if (r > 0) {
    memmove(buffer_save_a + r, buffer_save_a, (WIDTH_RAY - r) * sizeof(uint16_t));
    memset(buffer_save_a, 0, r * sizeof(uint16_t));     // Заповнення нулями
  }
}

/* Малювання старого графіка для каналу A */
void graph_ray_clean_a(uint16_t i)
{
  if (i >= WIDTH_RAY)
    i -= WIDTH_RAY;

  // display_signal(i, buffer_clean_a[i], i + 1, buffer_clean_a[i + 1], BLACK);
  DrawThickLine(i, buffer_clean_a[i], i + 1, buffer_clean_a[i + 1], 1, BLACK);
}

/* Малювання старого графіка для каналу B */
void graph_ray_clean_b(uint16_t i)
{
  if (i >= WIDTH_RAY)
    i -= WIDTH_RAY;

  // display_signal(i, buffer_clean_b[i], i + 1, buffer_clean_b[i + 1], BLACK);
  DrawThickLine(i, buffer_clean_b[i], i + 1, buffer_clean_b[i + 1], 1, BLACK);
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
      // display_signal(i, buffer_save_a[i], i + 1, buffer_save_a[i + 1], YELLOW);
      DrawThickLine(i, buffer_save_a[i], i + 1, buffer_save_a[i + 1], 1, YELLOW);

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
      // display_signal(i, buffer_save_b[i], i + 1, buffer_save_b[i + 1], GREEN);
      DrawThickLine(i, buffer_save_b[i], i + 1, buffer_save_b[i + 1], 1, GREEN);

    buffer_clean_b[i] = buffer_save_b[i];
  }
}

/* Малювання нового графіка для вибраного каналу */
void signal_draw(uint16_t *ptr_save, int16_t scroll_x, uint16_t i, uint32_t color)
{
  int index = scroll_x + i;
  if (index > WIDTH_RAY)
    index -= WIDTH_RAY;

  if (ptr_save == buffer_save_a) {
    graph_ray_clean_a(index + 1);       // Очищаємо графік для каналу A
    buffer_clean_a[i] = ptr_save[i];
  }

  if (ptr_save == buffer_save_b) {
    graph_ray_clean_b(index + 1);       // Очищаємо графік для каналу B
    buffer_clean_b[i] = ptr_save[i];
  }

  // не малюємо останню лінію. (позбуваємось сміття в кінці екрану)
  if ( index > (BEGIN_RAY + 2) && index < (WIDTH_RAY - 2) ) // не малюємо початок і кінець
    // display_signal(i, *(ptr_save + index), i + 1, *(ptr_save + index + 1), color);
    DrawThickLine(i, *(ptr_save + index), i + 1, *(ptr_save + index + 1), 1, color);
}

/* Малювання сітки точками по вертикалі */
void draw_vertical_grid(uint16_t i)
{
  int16_t step_x = i;

  // Малювання сітки точками по вертикалі
  for (uint16_t y = CELL_SIZE; y < MAX_Y_BORDER + 1; y += CELL_SIZE/5)
    for (uint16_t x = step_x; x < WIDTH_RAY; x += CELL_SIZE)
      DrawRectangle(x, y, 1, 1, WHITE);
}

/* Малювання сітки точками по горизонталі */
void draw_horisontal_grid(uint16_t i)
{
  int16_t step_x = i;

  // Малювання сітки точками по горизонталі
  for (uint16_t y = CELL_SIZE; y < MAX_Y_BORDER + 1; y += CELL_SIZE)
    for (uint16_t x = step_x; x < WIDTH_RAY; x += CELL_SIZE/5)
      DrawRectangle(x, y, 1, 1, WHITE);
}

