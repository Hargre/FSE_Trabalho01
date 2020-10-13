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


int main() {
    setlocale(LC_ALL, "");
    signal(SIGINT, sighandler);
    pthread_t temp_reading;
    pthread_t potentiometer_reading;
    pthread_t log_update;

    void *pthread_return;

    pthread_mutex_t mut;
    pthread_mutex_init(&mut, NULL);

    active_selection_mode = Potentiometer;
    readings.hysteresis_temperature = DEFAULT_HYSTERESIS;
    int in;

    initscr();
    show_menu(Potentiometer);

    pthread_create(&temp_reading, NULL, update_terminal_readings, &mut);
    pthread_create(&potentiometer_reading, NULL, update_reference_reading, NULL);
    pthread_create(&log_update, NULL, log_readings, NULL);


    in = getch();
    while (in != '0') {
        switch (in)
        {
        case '1':
            pthread_mutex_lock(&mut);
            if (active_selection_mode == Potentiometer) {
                pthread_cancel(potentiometer_reading);
                pthread_join(potentiometer_reading, &pthread_return);
            }
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
                pthread_create(&potentiometer_reading, NULL, update_reference_reading, NULL);
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