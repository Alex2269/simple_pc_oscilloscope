// moving_average_filter.h

#ifndef MOVING_AVERAGE_FILTER_H
#define MOVING_AVERAGE_FILTER_H

// Структура для фільтра ковзного середнього
typedef struct {
    int window_size;  // Розмір вікна усереднення (кількість значень у фільтрі)
    int *history;     // Вказівник на масив, який зберігає останні window_size значень
    int index;        // Поточний індекс у масиві history для запису нового значення (циклічний)
    int count;        // Кількість значень, які вже додані у фільтр (до window_size)
    long sum;         // Сума значень у поточному вікні (для швидкого обчислення середнього)
} MovingAverageFilter;

// Ініціалізація фільтра ковзного середнього
// filter - вказівник на структуру фільтра
// window_size - розмір вікна усереднення
void MovingAverageFilter_Init(MovingAverageFilter *filter, int window_size);

// Звільнення ресурсів, виділених для фільтра
void MovingAverageFilter_Free(MovingAverageFilter *filter);

// Обробка нового значення і обчислення поточного ковзного середнього
// new_value - нове вхідне значення
// Повертає: середнє значення за останні window_size елементів (або менше, якщо даних ще недостатньо)
int MovingAverageFilter_Process(MovingAverageFilter *filter, int new_value);

#endif // MOVING_AVERAGE_FILTER_H
