#include "stdint.h"
#include "test_signal.h"

// ======== test signal ==========
// #define SIN_TABLE_SIZE (WIDTH_RAY) // кількість точок на один цикл (1 фаза)
float phaseA[SIN_TABLE_SIZE], phaseB[SIN_TABLE_SIZE], phaseC[SIN_TABLE_SIZE];

// Параметри синусоїди
static float amplitudeA = 1.0f, amplitudeB = 1.0f, amplitudeC = 1.0f;
static float offsetA = 0.0f, offsetB = 0.0f, offsetC = 0.0f;
static float phaseShiftA = 0.0f, phaseShiftB = 120.0f, phaseShiftC = 240.0f;    // Зсув по фазі для кожної фази

// Функція для генерації синусоїд для кожної фази
void Generate_Sine_Waves(void)
{
  for (int i = 0; i < SIN_TABLE_SIZE; i++) {
    float angle = (i + phaseShiftA) * (PI / 180.0f);    // Переведення в радіани
    phaseA[i] = amplitudeA * sinf(angle) + offsetA;

    angle = (i + phaseShiftB) * (PI / 180.0f);
    phaseB[i] = amplitudeB * sinf(angle) + offsetB;

    angle = (i + phaseShiftC) * (PI / 180.0f);
    phaseC[i] = amplitudeC * sinf(angle) + offsetC;
  }
}

// ============================
