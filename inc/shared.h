#include <pthread.h>


#ifndef SHAREDVARS_H
#define SHAREDVARS_H

#define DEFAULT_HYSTERESIS 4.0

enum ReferenceModes {
    Potentiometer = 0,
    TerminalSelect
};

pthread_mutex_t uart_mutex;
struct readings_t {
    float external_temperature;
    float internal_temperature;
    float ref_temp;
    float hysteresis_temperature;
} readings;

enum ReferenceModes active_selection_mode;

void init_mutex();

#endif