#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO 22        // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO 21        // GPIO number for I2C master data
#define I2C_MASTER_NUM I2C_NUM_0    // I2C port number for master dev
#define I2C_MASTER_FREQ_HZ 400000   // I2C master clock frequency
#define I2C_MASTER_TX_BUF_DISABLE 0 // I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE 0 // I2C master doesn't need buffer

#define MS5837_ADDR 0x76 // MS5837-30BA I2C address

static int calibration_data[6] = {27919, 27937, 16806, 18058, 28925, 26652};

void i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void ms5837_reset()
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MS5837_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x1E, true); // Reset command
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(10 / portTICK_PERIOD_MS); // Wait for reset to complete
}

uint16_t ms5837_read_prom(uint8_t coef_num)
{
    uint8_t data[2];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MS5837_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xA0 + (coef_num * 2), true); // PROM read command
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MS5837_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data[0], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data[1], I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return (data[0] << 8) | data[1];
}

void ms5837_read_calibration_data(uint16_t *calibration_data)
{
    for (int i = 0; i < 6; i++)
    {
        calibration_data[i] = ms5837_read_prom(i + 1);
    }
    printf("C1: %d, C2: %d, C3: %d, C4: %d, C5: %d, C6: %d\n", calibration_data[0], calibration_data[1], calibration_data[2], calibration_data[3], calibration_data[4], calibration_data[5]);
}

uint32_t ms5837_read_adc(uint8_t cmd)
{
    uint8_t data[3];
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MS5837_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, cmd, true); // Start conversion command
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    vTaskDelay(10 / portTICK_PERIOD_MS); // Wait for conversion to complete

    cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MS5837_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x00, true); // ADC read command
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (MS5837_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd_handle, &data[0], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd_handle, &data[1], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd_handle, &data[2], I2C_MASTER_NACK);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd_handle, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd_handle);

    return ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | data[2];
}

uint32_t ms5837_read_pressure()
{
    return ms5837_read_adc(0x48); // D1 conversion command
}

uint32_t ms5837_read_temperature()
{
    return ms5837_read_adc(0x58); // D2 conversion command
}

void ms5837_calculate(uint32_t D1, uint32_t D2, float *pressure, float *temperature)
{
    int32_t dT = D2 - ((uint32_t)calibration_data[4] << 8);
    int32_t TEMP = 2000 + ((int64_t)dT * calibration_data[5] >> 23);

    int64_t OFF = ((int64_t)calibration_data[1] << 16) + ((int64_t)calibration_data[3] * dT >> 7);
    int64_t SENS = ((int64_t)calibration_data[0] << 15) + ((int64_t)calibration_data[2] * dT >> 8);

    if (TEMP < 2000)
    {
        int32_t T2 = (dT * dT) >> 31;
        int64_t OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) >> 1;
        int64_t SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) >> 2;

        if (TEMP < -1500)
        {
            OFF2 += 7 * ((TEMP + 1500) * (TEMP + 1500));
            SENS2 += 11 * ((TEMP + 1500) * (TEMP + 1500)) >> 1;
        }

        TEMP -= T2;
        OFF -= OFF2;
        SENS -= SENS2;
    }

    *pressure = (float)(((D1 * SENS >> 21) - OFF) / 8192.0f / 100.0f); // Convert to mbar
    *temperature = (float)TEMP / 100.0f;                               // Convert to Celsius
}