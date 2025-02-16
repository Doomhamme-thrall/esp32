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
#define MS5837_I2C_MASTER_SCL_IO 22        // GPIO number for I2C master clock
#define MS5837_I2C_MASTER_SDA_IO 21        // GPIO number for I2C master data
#define MS5837_I2C_MASTER_NUM I2C_NUM_0    // I2C port number for master dev
#define MS5837_I2C_MASTER_FREQ_HZ 400000   // I2C master clock frequency
#define MS5837_I2C_MASTER_TX_BUF_DISABLE 0 // I2C master doesn't need buffer
#define MS5837_I2C_MASTER_RX_BUF_DISABLE 0 // I2C master doesn't need buffer

// MS5837-30BA I2C Address
#define MS5837_ADDR 0x76 // MS5837-30BA I2C address

// MS5837 Commands
#define MS5837_RESET_CMD 0x1E      // Reset command
#define MS5837_ADC_READ_CMD 0x00   // ADC read command
#define MS5837_PROM_READ_CMD 0xA0  // Base address for PROM read
#define MS5837_CONVERT_D1_CMD 0x48 // D1 (pressure) conversion command
#define MS5837_CONVERT_D2_CMD 0x58 // D2 (temperature) conversion command

// Calibration coefficients count
#define MS5837_CALIBRATION_COEFF_COUNT 6 // Number of calibration coefficients

    /**
     * @brief Initialize the I2C master interface for MS5837-30BA.
     */
    void i2c_master_init(void);

    /**
     * @brief Reset the MS5837-30BA sensor.
     */
    void ms5837_reset(void);

    /**
     * @brief Read a calibration coefficient from the PROM.
     *
     * @param coef_num Coefficient number (1 to 6).
     * @return uint16_t The calibration coefficient value.
     */
    uint16_t ms5837_read_prom(uint8_t coef_num);

    /**
     * @brief Read all calibration coefficients from the PROM.
     *
     * @param calibration_data Array to store the calibration coefficients.
     */
    void ms5837_read_calibration_data(uint16_t *calibration_data);

    /**
     * @brief Read the ADC value for pressure or temperature.
     *
     * @param cmd The conversion command (D1 or D2).
     * @return uint32_t The ADC value.
     */
    uint32_t ms5837_read_adc(uint8_t cmd);

    /**
     * @brief Read the raw pressure value from the sensor.
     *
     * @return uint32_t The raw pressure value (D1).
     */
    uint32_t ms5837_read_pressure(void);

    /**
     * @brief Read the raw temperature value from the sensor.
     *
     * @return uint32_t The raw temperature value (D2).
     */
    uint32_t ms5837_read_temperature(void);

    /**
     * @brief Calculate the compensated pressure and temperature.
     *
     * @param calibration_data Array of calibration coefficients.
     * @param D1 Raw pressure value.
     * @param D2 Raw temperature value.
     * @param pressure Pointer to store the compensated pressure (in mbar).
     * @param temperature Pointer to store the compensated temperature (in Celsius).
     */
    void ms5837_calculate(uint32_t D1, uint32_t D2, float *pressure, float *temperature);

#ifdef __cplusplus
}
#endif

#endif // MS5837_30BA_H