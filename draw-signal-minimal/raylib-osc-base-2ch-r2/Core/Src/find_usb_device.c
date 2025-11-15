// file find_usb_device.h

// #include "main.h"
#include "rs232.h"
#include "find_usb_device.h"
#include <stdint.h>
#include <stdbool.h>

int find_usb_device(void)
{
    char com_port_name_input[20]; // Ім'я COM-порту, введене користувачем
    int comport_number;           // Індекс відкритого COM-порту (-1 якщо не відкрито)
    // Автоматичне відкриття COM-порту
    int comport_to_find = 0;
    char mode[] = {'8','N','1',0};
    bool port_found = false;

    while (comport_to_find < 38) {
        if (RS232_OpenComport(comport_to_find, 115200, mode, 0) == 0) {
            comport_number = comport_to_find;
            printf("Автоматично відкрито COM порт: %d\n", comport_number);
            port_found = true;
            // if (comport_to_find == 24) {
            //     strcpy(com_port_name_input, "/dev/ttyACM0");
            // } else {
            //     sprintf(com_port_name_input, "Port %d", comport_to_find);
            // }
            // break;
        }
        comport_to_find++;
        if (comport_to_find == 37) {
            sleep(1);
        }
    }

    if (!port_found) {
        printf("Не вдалося автоматично відкрити жоден COM порт.\n");
        strcpy(com_port_name_input, "/dev/ttyACM0");
    }
    return comport_number;
}

