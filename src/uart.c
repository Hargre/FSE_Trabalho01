#include "uart.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

float send_message(unsigned char *message, int size, unsigned char code) {
    int uart_stream = -1;

    uart_stream = open(SERIAL_PATH, O_RDWR | O_NOCTTY | O_NDELAY);

    if (uart_stream == -1) {
        return -1;
    }

    struct termios options;
    tcgetattr(uart_stream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    tcflush(uart_stream, TCIFLUSH);

    tcsetattr(uart_stream, TCSANOW, &options);

    int bytes_written = write(uart_stream, message, size);

    usleep(20000);

    float data = read_data(uart_stream);
    close(uart_stream);
    return data;
}


float request_data(unsigned char code) {
    unsigned char message[5];
    unsigned char *message_ptr = &message[0];
    *message_ptr++ = code;
    *message_ptr++ = 9;
    *message_ptr++ = 3;
    *message_ptr++ = 1;
    *message_ptr++ = 3;
    return send_message(message, (message_ptr - &message[0]), code);
}


float read_data(int stream) {
    int length = -1;
    float sensor_reading;

    length = read(stream, (void *)&sensor_reading, sizeof(float));

    return sensor_reading;
}
