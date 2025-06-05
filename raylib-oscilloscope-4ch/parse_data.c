// parse_data.c

#include <string.h> // strstr
#include <stdlib.h> // atoi
#include "parse_data.h"

void parse_data(char *str, int *value_a, int *value_b)
{
    const char *pa0_ptr = strstr(str, "PA0 ");
    const char *pa1_ptr = strstr(str, "PA1 ");

    if (pa0_ptr)
    {
        pa0_ptr += 4; // Пропускаємо "PA0 "
        char buffer[10] = {0};
        int i = 0;
        while (pa0_ptr[i] != '$' && pa0_ptr[i] != '\0' && i < (int)(sizeof(buffer) - 1))
        {
            buffer[i] = pa0_ptr[i];
            i++;
        }
        buffer[i] = '\0';
        *value_a = atoi(buffer);
    }
    else
    {
        *value_a = -1; // Якщо не знайдено
    }

    if (pa1_ptr)
    {
        pa1_ptr += 4; // Пропускаємо "PA1 "
        char buffer[10] = {0};
        int i = 0;
        while (pa1_ptr[i] != '$' && pa1_ptr[i] != '\0' && i < (int)(sizeof(buffer) - 1))
        {
            buffer[i] = pa1_ptr[i];
            i++;
        }
        buffer[i] = '\0';
        *value_b = atoi(buffer);
    }
    else
    {
        *value_b = -1; // Якщо не знайдено
    }
}
