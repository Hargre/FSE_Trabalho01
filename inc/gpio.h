#ifndef GPIOC_H
#define GPIOC_H

void setup_gpio();
void toggle_resistor(int status);
void toggle_fan(int status);
void close_gpio();

#endif