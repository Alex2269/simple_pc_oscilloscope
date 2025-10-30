#ifndef TEST_SIGNAL_H
#define TEST_SIGNAL_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "osc.h"

#define PI 3.14159265358979
#define SIN_TABLE_SIZE (WIDTH_RAY) // кількість точок на один цикл (1 фаза)

// Масиви для трифазних синусоїд
extern float phaseA[], phaseB[], phaseC[];

// Функція для генерації синусоїд для кожної фази
void Generate_Sine_Waves(void);


#ifdef __cplusplus
}
#endif

#endif /* TEST_SIGNAL_H */
