#include "shared.h"

void init_mutex() {
    pthread_mutex_init(&uart_mutex, NULL);
}