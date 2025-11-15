#define PACKET_SIZE 13

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

int open_serial(const char *device) {
    int fd = open(device, O_RDONLY | O_NOCTTY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    // Налаштування швидкості порту (115200, наприклад)
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // 8N1, без контролю потоку
    tty.c_cflag &= ~PARENB; // Без парності
    tty.c_cflag &= ~CSTOPB; // 1 стоп-біт
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     // 8 бітів
    tty.c_cflag &= ~CRTSCTS; // Без апаратного контролю потоку
    tty.c_cflag |= CREAD | CLOCAL; // Увімкнути прийом

    // Відключити канонічний режим, ехо, сигнали
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Відключити програмний контроль потоку
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Відключити обробку вихідних символів
    tty.c_oflag &= ~OPOST;

    // Таймаути для читання
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10; // 1 секунда таймаут

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

void process_packet(uint8_t *packet) {
    printf("Received packet:\n");
    for (int i = 1; i < PACKET_SIZE; i += 3) {
        uint8_t channel_id = packet[i];
        uint16_t value = packet[i+1] | (packet[i+2] << 8);
        printf("Channel %d: %d\n", channel_id, value);
    }
}

int main() {
    int fd = open_serial("/dev/ttyACM0");
    if (fd < 0) return 1;

    uint8_t buffer[PACKET_SIZE];
    int idx = 0;

    while (1) {
        uint8_t byte;
        int n = read(fd, &byte, 1);
        if (n < 0) {
            perror("read");
            break;
        } else if (n == 0) {
            // Таймаут, можна продовжувати
            continue;
        }

        if (idx == 0) {
            if (byte == 0xAA) { // шукаємо стартовий байт
                buffer[idx++] = byte;
            }
        } else {
            buffer[idx++] = byte;
            if (idx == PACKET_SIZE) {
                process_packet(buffer);
                idx = 0;
            }
        }
    }

    close(fd);
    return 0;
}
