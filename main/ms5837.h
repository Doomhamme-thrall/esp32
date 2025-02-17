#ifndef MS5837_30BA_H
#define MS5837_30BA_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif

// I2C Configuration
#define MS5837_I2C_MASTER_SCL_IO 22
#define MS5837_I2C_MASTER_SDA_IO 21
#define MS5837_I2C_MASTER_NUM I2C_NUM_0
#define MS5837_I2C_MASTER_FREQ_HZ 400000
#define MS5837_I2C_MASTER_TX_BUF_DISABLE 0
#define MS5837_I2C_MASTER_RX_BUF_DISABLE 0

// MS5837 I2C Address
#define MS5837_ADDR 0x76

// MS5837 Commands
#define MS5837_RESET_CMD 0x1E
#define MS5837_ADC_READ_CMD 0x00
#define MS5837_PROM_READ_CMD 0xA0
#define MS5837_CONVERT_D1_CMD 0x48
#define MS5837_CONVERT_D2_CMD 0x58

#define MS5837_CALIBRATION_COEFF_COUNT 6

    void i2c_master_init(void);
    void ms5837_reset(void);
    uint16_t ms5837_read_prom(uint8_t coef_num);
    void ms5837_read_calibration_data(uint16_t *calibration_data);
    uint32_t ms5837_read_adc(uint8_t cmd);
    uint32_t ms5837_read_pressure(void);
    uint32_t ms5837_read_temperature(void);
    void ms5837_calculate(uint32_t D1, uint32_t D2, float *pressure, float *temperature);
    void ms5837_get_data(float *pressure, float *temperature);

#ifdef __cplusplus
}
#endif

#endif // MS5837_30BA_H