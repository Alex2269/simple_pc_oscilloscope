#ifndef RANDOM_H
#define RANDOM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdlib.h>
#include <time.h>

// Функція для ініціалізації генератора випадкових чисел
void init_random(void);
// Функція для генерування випадкових чисел в діапазоні від 0 до 4095
int random_in_range(void);


#ifdef __cplusplus
}
#endif

#endif /* RANDOM_H */
