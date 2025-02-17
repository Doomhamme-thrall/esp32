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
#include "uart.h"

// unix时间戳
struct timeval time_unix = {
    .tv_sec = 1739685416,
    .tv_usec = 0};

// 深度数据
float depth_data[1024] = {0};
// 对应的时间
int unix_time[1024] = {0};

int report_flag = 0;

void app_main()
{
    i2c_master_init();
    ms5837_reset();
    uart_init();

    while (1)
    {
        uart_write_bytes(UART_NUM_1, "hello", 5);
        printf("%d,%d", cmd.start, cmd.unix_time);
        report_flag = cmd.start;
        int index = 0;
        while (cmd.start == 1)
        {
            ms5837_get_data(&depth_data[index], NULL);
            unix_time[index] = time(NULL);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        if (report_flag == 1)
        {
            for (int i = 0; i < index; i++)
            {
                printf("%d,%f\n", unix_time[i], depth_data[i]);
            }
            report_flag = 0;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

//PID定深？测定水下浮动大小