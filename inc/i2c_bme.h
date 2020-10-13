#ifndef I2C_BME_H
#define I2C_BME_H

#include "bme280_defs.h"

struct identifier {
    uint8_t dev_addr;
    int8_t fd;
};

void delay_us(uint32_t period, void *intf_ptr);
int8_t i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);
int8_t i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);
int8_t calibrate_sensor(struct bme280_dev *dev);
double get_sensor_data();
float read_sensor(struct bme280_dev *dev);


#endif