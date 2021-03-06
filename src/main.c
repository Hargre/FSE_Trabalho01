#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <locale.h>

#include <bcm2835.h>

#include "uart.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include "gpio.h"

#include "shared.h"
#include "interface.h"


void cleanup() {
    setup_gpio();
    toggle_resistor(HIGH);
    toggle_fan(HIGH);
    close_gpio();
}

void sighandler(int signum) {
    cleanup();
    exit(0);
}

int log_counter = 0;

void alarmhandler(int sigalarm) {
    log_counter++;

    pthread_mutex_lock(&temp_readings_mutex);
    temp_readings_flag = 1;
    pthread_cond_signal(&temp_readings_cond);
    pthread_mutex_unlock(&temp_readings_mutex);

    if (log_counter == 4) {
        log_counter = 0;
        pthread_mutex_lock(&logs_mutex);
        logs_flag = 1;
        pthread_cond_signal(&logs_cond);
        pthread_mutex_unlock(&logs_mutex);
    }
    ualarm(500000, 0);
}


int main() {
    setlocale(LC_ALL, "");
    signal(SIGINT, sighandler);
    signal(SIGALRM, alarmhandler);
    pthread_t temp_reading;
    pthread_t potentiometer_reading;
    pthread_t log_update;

    void *pthread_return;

    pthread_mutex_t mut;
    temp_readings_flag = 0;
    logs_flag = 0;
    pthread_mutex_init(&mut, NULL);
    pthread_mutex_init(&temp_readings_mutex, NULL);
    pthread_cond_init(&temp_readings_cond, NULL);

    pthread_mutex_init(&logs_mutex, NULL);
    pthread_cond_init(&logs_cond, NULL);

    active_selection_mode = Potentiometer;
    readings.hysteresis_temperature = DEFAULT_HYSTERESIS;
    int in;

    initscr();
    show_menu(Potentiometer);

    pthread_create(&temp_reading, NULL, update_terminal_readings, &mut);
    pthread_create(&log_update, NULL, log_readings, NULL);

    ualarm(500000, 0);

    in = getch();
    while (in != '0') {
        switch (in)
        {
        case '1':
            pthread_mutex_lock(&mut);
            erase();
            mvprintw(2, 0, "Insira o valor de referência:");
            refresh();
            mvscanw(3, 0, "%f", &readings.ref_temp);
            active_selection_mode = TerminalSelect;
            show_menu(active_selection_mode);
            pthread_mutex_unlock(&mut);
            break;
        case '2':
            if (active_selection_mode == TerminalSelect) {
                pthread_mutex_lock(&mut);
                active_selection_mode = Potentiometer;
                show_menu(active_selection_mode);
                pthread_mutex_unlock(&mut);
            }
            break;
        case '3':
            pthread_mutex_lock(&mut);
            erase();
            mvprintw(2, 0, "Insira o valor de histerese:");
            refresh();
            mvscanw(3, 0, "%f", &readings.hysteresis_temperature);
            show_menu(active_selection_mode);
            pthread_mutex_unlock(&mut);
            break;
        default:
            break;
        }
        in = getch();
    }
    
    endwin();
    cleanup();

    return 0;
}