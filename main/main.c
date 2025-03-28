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

#define target 200       // 目标深度
#define data_size 10240  // 数据大小
#define stepper_max 4500 // 丝杆极限

float depth_data[data_size] = {0}; // 深度数据
int unix_time[data_size] = {0};    // 对应的时间
int reached_time = 0;              // 到达目标深度的时间
extern TaskHandle_t uart_task_handle;
int index = 0;
int steps_moved = 0;

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

// void app_main()
// {
//     i2c_master_init();
//     ms5837_reset();
//     uart_init();
//     stepper_init();
//     printf("all ready\n");
//     uart_write_bytes(UART_NUM_1, "all ready", 9);

//     stepper_move(100);
//     stepper_move(-100);
//     while (1)
//     {
//         if (cmd.start)
//         {
//             stepper_move(cmd.unix_time);
//             cmd.unix_time = 0;
//             cmd.start = 0;
//         }
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

void app_main()
{
    // 初始化
    i2c_master_init();
    ms5837_reset();
    uart_init();
    stepper_init();

    // 等待开始指令
    printf("all ready\n");
    uart_write_bytes(UART_NUM_1, "all ready", 9);

    State state = wait;

    while (1)
    {
        switch (state)
        {
        case init:
            time_sync(cmd.unix_time);
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
                stepper_move(-500);
                steps_moved -= 500;
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
                stepper_move(-100);
                steps_moved -= 100;
            }
            else
            {
                stepper_move(100);
                steps_moved += 100;
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
            stepper_move(-steps_moved);
            state = report;
            break;
        // 回传
        case report:
            for (int i = 0; i < index; i++)
            {
                uart_write_bytes(UART_NUM_1, &unix_time[i], sizeof(int));
                uart_write_bytes(UART_NUM_1, &depth_data[i], sizeof(float));
            }
            cmd.start = 0;
            state = wait;
            break;
        // 待机
        case wait:
            if (cmd.start == 1)
            {
                state = dowm;
                printf("all ready2\n");
                uart_write_bytes(UART_NUM_1, "all ready2", 8);
            }
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
