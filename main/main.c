#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <sys/time.h>
#include "driver/i2c.h"

#include "pwm.h"
#include "i2c.h"
#include "ms5837.h"
#include "get_time.h"

// unix时间戳
struct timeval time_unix = {
    .tv_sec = 1739685416,
    .tv_usec = 0};

// 深度数据
float depth_data[1024] = {0};
// 对应的时间
int unix_time[1024] = {0};

void app_main()
{
    i2c_master_init();
    ms5837_reset();

    // uint16_t calibration_data[6];
    // ms5837_read_calibration_data(calibration_data);

    while (1)
    {
        uint32_t D1 = ms5837_read_pressure();
        uint32_t D2 = ms5837_read_temperature();

        float pressure, temperature;
        ms5837_calculate(D1, D2, &pressure, &temperature);

        printf("%.2f , %.2f \n", pressure, temperature);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
