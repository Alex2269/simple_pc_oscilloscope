#include <stdio.h>
#include "moving_average_filter.h"

int main() {
    // Оголошуємо змінну фільтра
    MovingAverageFilter filter;

    // Ініціалізуємо фільтр з вікном розміром 5
    MovingAverageFilter_Init(&filter, 5);

    // Масив вхідних даних (наприклад, виміри з датчика)
    int input_values[] = {10, 20, 30, 40, 50, 60, 70};
    int n = sizeof(input_values) / sizeof(input_values[0]);

    printf("Вхідні значення\tКовзне середнє\n");

    // Обробляємо кожне значення через фільтр
    for (int i = 0; i < n; i++) {
        int filtered_value = MovingAverageFilter_Process(&filter, input_values[i]);
        printf("%d\t\t%d\n", input_values[i], filtered_value);
    }

    // Звільняємо пам'ять, виділену під фільтр
    MovingAverageFilter_Free(&filter);

    return 0;
}

