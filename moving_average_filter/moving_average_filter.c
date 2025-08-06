// moving_average_filter.c

#include "moving_average_filter.h"
#include <stdlib.h>
#include <string.h>

// Ініціалізація фільтра ковзного середнього
// filter - вказівник на структуру фільтра
// window_size - розмір вікна усереднення (кількість значень для усереднення)
void MovingAverageFilter_Init(MovingAverageFilter *filter, int window_size) {
    filter->window_size = window_size; // Запам'ятовуємо розмір вікна

    // Виділяємо пам'ять під масив історії значень (останніх window_size елементів)
    filter->history = (int*)malloc(sizeof(int) * window_size);

    // Заповнюємо масив нулями, щоб уникнути сміття у пам'яті
    memset(filter->history, 0, sizeof(int) * window_size);

    filter->index = 0;  // Індекс для циклічного запису нових значень у масив history
    filter->count = 0;  // Лічильник кількості доданих значень (до window_size)
    filter->sum = 0;    // Сума значень у поточному вікні для швидкого обчислення середнього
}

// Звільнення ресурсів, виділених для фільтра
void MovingAverageFilter_Free(MovingAverageFilter *filter) {
    if (filter->history != NULL) {
        free(filter->history);    // Звільняємо пам'ять, виділену під масив history
        filter->history = NULL;   // Обнуляємо вказівник для безпеки
    }
}

// Обробка нового вхідного значення і обчислення ковзного середнього
// new_value - нове значення, яке додається у фільтр
// Повертає: поточне усереднене значення (int)
int MovingAverageFilter_Process(MovingAverageFilter *filter, int new_value) {
    // Оновлюємо суму: віднімаємо старе значення, яке замінюємо, і додаємо нове
    filter->sum = filter->sum - filter->history[filter->index] + new_value;

    // Записуємо нове значення у масив історії на позицію index
    filter->history[filter->index] = new_value;

    // Оновлюємо індекс циклічно (по колу в межах window_size)
    filter->index = (filter->index + 1) % filter->window_size;

    // Збільшуємо лічильник, якщо він ще не досяг розміру вікна
    if (filter->count < filter->window_size) filter->count++;

    // Обчислюємо і повертаємо середнє значення (цілочисельне ділення)
    return (int)(filter->sum / filter->count);
}

