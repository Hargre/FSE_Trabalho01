#include "interface.h"
#include "gpio.h"
#include "readings.h"
#include "i2c_lcd.h"
#include "shared.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>


void *update_terminal_readings(void *mutex) {
    start_lcd_display();

    while (1) {
        pthread_mutex_lock(&temp_readings_mutex);
        while (!temp_readings_flag) {
            pthread_cond_wait(&temp_readings_cond, &temp_readings_mutex);
        }
        pthread_mutex_unlock(&temp_readings_mutex);

        get_internal_temperature();
        get_external_temperature();

        if (active_selection_mode == Potentiometer) {
            get_potentiometer_reference_temperature();
        }

        setup_gpio();
        pthread_mutex_lock(&uart_mutex);
        if (readings.internal_temperature < readings.ref_temp - readings.hysteresis_temperature) {
            toggle_resistor(LOW);
            toggle_fan(HIGH);
        } else if (readings.internal_temperature > readings.ref_temp + readings.hysteresis_temperature ) {
            toggle_fan(LOW);
            toggle_resistor(HIGH);
        } else {
            toggle_resistor(HIGH);
            toggle_fan(HIGH);
        }
        pthread_mutex_unlock(&uart_mutex);
        close_gpio();

        pthread_mutex_lock((pthread_mutex_t *)mutex);
        mvprintw(
            0,0,"TI: %.2f TE: %.2f TR: %.2f H: %.2f\n",
            readings.internal_temperature,
            readings.external_temperature,
            readings.ref_temp,
            readings.hysteresis_temperature
        );
        update_lcd_display();
        refresh();
        pthread_mutex_unlock((pthread_mutex_t *)mutex);

        pthread_mutex_lock(&temp_readings_mutex);
        temp_readings_flag = 0;
        pthread_mutex_unlock(&temp_readings_mutex);
    }
    return NULL;
}

void *log_readings() {

    while(1) {
        pthread_mutex_lock(&logs_mutex);
        while (!logs_flag) {
            pthread_cond_wait(&logs_cond, &logs_mutex);
        }
        pthread_mutex_unlock(&logs_mutex);
        save_readings_log();
        pthread_mutex_lock(&logs_mutex);
        logs_flag = 0;
        pthread_mutex_unlock(&logs_mutex);
    }
}

void start_lcd_display() {
    wiringPiSetup();
    lcd_fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init();
}

void update_lcd_display() {
    lcdLoc(LINE1);
    typeln("TI: ");
    typeFloat(readings.internal_temperature);
    typeln(" TE: ");
    typeFloat(readings.external_temperature);
    lcdLoc(LINE2);
    typeln("TR: " );
    typeFloat(readings.ref_temp);
}

void show_menu(int selection) {
    erase();
    mvprintw(
        0,0,"TI: %.2f TE: %.2f TR: %.2f H: %.2f\n",
        readings.internal_temperature,
        readings.external_temperature,
        readings.ref_temp,
        readings.hysteresis_temperature
    );
    mvprintw(2, 0, "Modo de Seleção atual: %s", (selection == Potentiometer ? "Potenciômetro" : "Terminal"));
    mvprintw(3, 0, "1 - Definir Temperatura de Referência");
    mvprintw(4, 0, "2 - Controlar Potenciômetro");
    mvprintw(5, 0, "3 - Definir Valor de Histerese");
    mvprintw(6, 0, "0 - Sair");
    refresh();
}