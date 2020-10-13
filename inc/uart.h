#ifndef UART_H
#define UART_H

#define SERIAL_PATH "/dev/serial0"

#define REQUEST_INTERNAL_TEMP 0xA1
#define REQUEST_REF_TEMP 0xA2

float send_message(unsigned char *message, int size, unsigned char code);
float request_data(unsigned char code);
float read_data(int stream);

#endif