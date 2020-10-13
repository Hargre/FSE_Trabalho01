#include "readings.h"
#include "uart.h"
#include "i2c_bme.h"
#include "shared.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "shared.h"


float get_internal_temperature() {
    pthread_mutex_lock(&uart_mutex);
    readings.internal_temperature = request_data(REQUEST_INTERNAL_TEMP);
    pthread_mutex_unlock(&uart_mutex);
}

float get_external_temperature() {
    pthread_mutex_lock(&uart_mutex);
    readings.external_temperature = get_sensor_data();
    pthread_mutex_unlock(&uart_mutex);
}

float get_potentiometer_reference_temperature() {
    pthread_mutex_lock(&uart_mutex);
    readings.ref_temp = request_data(REQUEST_REF_TEMP);
    pthread_mutex_unlock(&uart_mutex);
}

void save_readings_log() {
    FILE *log = fopen("log.csv", "a+");
    fseek(log, 0, SEEK_END);
    if (ftell(log) == 0) {
        fseek(log, 0, SEEK_SET);
        fprintf(log, "horario, ti, te, tr\n");
    }

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char timestring[64];
    strftime(timestring, sizeof(timestring), "%c", tm);
    pthread_mutex_lock(&uart_mutex);
    fprintf(log, "%s, %0.2lf, %0.2lf, %0.2lf\n", timestring, readings.internal_temperature, readings.external_temperature, readings.ref_temp);
    pthread_mutex_unlock(&uart_mutex);
    fclose(log);
}