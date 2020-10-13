#include "i2c_bme.h"
#include "bme280.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

void delay_us(uint32_t period, void *intf_ptr) {
    usleep(period);
}

int8_t i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

int8_t i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
    uint8_t *buf;
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    buf = malloc(len + 1);
    buf[0] = reg_addr;
    memcpy(buf + 1, data, len);
    if (write(id.fd, buf, len + 1) < (uint16_t)len) {
        return BME280_E_COMM_FAIL;
    }

    free(buf);

    return BME280_OK;
}

int8_t calibrate_sensor(struct bme280_dev *dev) {
    int8_t result = BME280_OK;
    uint8_t settings_sel = 0;

    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    result = bme280_set_sensor_settings(settings_sel, dev);
    return result;
}

double get_sensor_data() {
    struct bme280_dev dev;
    struct identifier id;

    int8_t result = BME280_OK;

    id.dev_addr = BME280_I2C_ADDR_PRIM;
    id.fd = open("/dev/i2c-1", O_RDWR);

    if (id.fd < 0) {
        fprintf(stderr, "Erro ao tentar abrir o barramento\n");
        return -1;
    }

    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0){
        fprintf(stderr, "Erro ao se comunicar com o barramento.\n");
        return -1;
    }

    dev.intf = BME280_I2C_INTF;
    dev.read = i2c_read;
    dev.write = i2c_write;
    dev.delay_us = delay_us;

    dev.intf_ptr = &id;

    result = bme280_init(&dev);
    if (result != BME280_OK) {
        fprintf(stderr, "Erro ao inicializar o dispositivo. Código %+d\n", result);
        return -1;
    }

    float temp = read_sensor(&dev);

    close(id.fd);
    return temp;
}

float read_sensor(struct bme280_dev *dev) {
    int8_t result = BME280_OK;
    uint32_t req_delay;

    struct bme280_data data;

    result = calibrate_sensor(dev);
    if (result != BME280_OK) {
        fprintf(stderr, "Erro ao configurar sensor. Código %+d\n", result);
        return result;
    }

    req_delay = bme280_cal_meas_delay(&dev->settings);


    result = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
    if (result != BME280_OK) {
        fprintf(stderr, "Erro ao definir modo do sensor. Código %+d\n", result);
        return -1;
    }

    dev->delay_us(req_delay, dev->intf_ptr);
    usleep(50000);
    result = bme280_get_sensor_data(BME280_ALL, &data, dev);
    if (result != BME280_OK) {
        fprintf(stderr, "Erro ao ler dados do sensor. Código %+d\n", result);
        return -1;
    }

    return (&data)->temperature;
}