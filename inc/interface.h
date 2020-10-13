#ifndef INTERFACE_H
#define INTERFACE_H

void *update_terminal_readings(void *mutex);
void *log_readings();
void *update_reference_reading();
void start_lcd_display();
void update_lcd_display();
void show_menu(int selection);

#endif