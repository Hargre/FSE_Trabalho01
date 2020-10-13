#include <stdio.h>
#include "gpio.h"

#include <bcm2835.h>


#define RESISTOR_PIN RPI_GPIO_P1_16
#define FAN_PIN RPI_GPIO_P1_18
 
void setup_gpio() {
    bcm2835_init();
    bcm2835_gpio_fsel(RESISTOR_PIN, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(FAN_PIN, BCM2835_GPIO_FSEL_OUTP);
}

void toggle_resistor(int status) {
    bcm2835_gpio_write(RESISTOR_PIN, status);
}

void toggle_fan(int status) {
    bcm2835_gpio_write(FAN_PIN, status);
}

void close_gpio() {
    bcm2835_close();
}
 
