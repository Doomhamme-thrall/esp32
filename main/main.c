#include <stdio.h>
#include <inttypes.h>
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
#include "esp_task_wdt.h"
#include "esp_task.h"

#include "pwm.h"
#include "i2c.h"
#include "ms5837.h"
#include "get_time.h"
#include "uart.h"
#include "stepper.h"

#define target 200      // 目标深度
#define data_size 10240 // 数据大小

// unix时间戳
struct timeval time_unix = {
    .tv_sec = 0,
    .tv_usec = 0};

float depth_data[data_size] = {0}; // 深度数据
int unix_time[data_size] = {0};    // 对应的时间
int reached_time = 0;              // 到达目标深度的时间

int index = 0;

// 状态机
typedef enum
{
    init,
    dowm,
    keep,
    up,
    report,
    wait
} State;

void app_main()
{
    // 初始化
    i2c_master_init();
    ms5837_reset();
    uart_init();
    stepper_init();

    // 等待开始指令
    printf("started\n");
    uart_write_bytes(UART_NUM_1, "started", 7);

    State state = wait;

    while (1)
    {

        switch (state)
        {
        case init:
            struct timeval tv;
            tv.tv_sec = cmd.unix_time;
            tv.tv_usec = 0;
            settimeofday(&tv, NULL); // 初始化时间

            state = dowm;
            break;

        // 下潜
        case dowm:
            ms5837_get_data(&depth_data[index], NULL);
            unix_time[index] = time(NULL);
            printf("depth: %f\n", depth_data[index]);
            printf("time: %d\n", unix_time[index]);
            if (depth_data[index] < target)
            {
                stepper_move(-100);
            }
            else
            {
                reached_time = time(NULL);
                state = keep;
            }
            index++;
            break;

        // 定深
        case keep:
            ms5837_get_data(&depth_data[index], NULL);
            unix_time[index] = time(NULL);
            printf("depth: %f\n", depth_data[index]);
            printf("time: %d\n", unix_time[index]);
            if (depth_data[index] < target)
            {
                stepper_move(-10);
            }
            else
            {
                stepper_move(10);
            }
            if (time(NULL) - reached_time > 30)
            {
                reached_time = 0;
                state = up;
            }
            break;

        // 上浮
        case up:
            ms5837_get_data(&depth_data[index], NULL);
            unix_time[index] = time(NULL);
            printf("depth: %f\n", depth_data[index]);
            printf("time: %d\n", unix_time[index]);
            if (depth_data[index] >= 0.1)
            {
                state = report;
            }
            else
            {
                stepper_move(100);
            }
            break;

        case report:
            for (int i = 0; i < index; i++)
            {
                uart_write_bytes(UART_NUM_1, &unix_time[i], sizeof(int));
                uart_write_bytes(UART_NUM_1, &depth_data[i], sizeof(float));
            }
            state = wait;
            break;
        // 待机
        case wait:
            cmd.start = 0;
            if (cmd.start == 1)
            {
                state = init;
                printf("started2\n");
                uart_write_bytes(UART_NUM_1, "started2", 8);
            }
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
