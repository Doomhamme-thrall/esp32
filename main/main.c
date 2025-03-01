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

#include "pwm.h"
#include "i2c.h"
#include "ms5837.h"
#include "get_time.h"
#include "uart.h"
#include "stepper.h"

// unix时间戳
struct timeval time_unix = {
    .tv_sec = 1739685416,
    .tv_usec = 0};

// 深度数据
float depth_data[1024] = {0};
// 对应的时间
int unix_time[1024] = {0};

int report_flag = 0;
int reached_time = 0;

int steps = 0;

int index = 0;

// 定深pid
void depth_pid(float depth, float target)
{
    float Kp = 0.1;
    float Ki = 0.1;
    float Kd = 0.1;
    float prev_time = 0;
    float dt = 0;
    float integral = 0;
    float derivative = 0;
    float prev_error = 0;
    float error = 0;
    dt = (time_unix.tv_usec - prev_time) * 1000; // 待改用定时器
    error = target - depth;
    integral += error * dt;
    derivative = (error - prev_error) / dt;
    stepper_move(Kp * error + Ki * integral + Kd * derivative);
    prev_error = error;
    prev_time = time_unix.tv_usec;
}

void app_main()
{
    // 初始化
    // i2c_master_init();
    // ms5837_reset();
    // uart_init();

    stepper_init();
    stepper_move(100);
    // 等待指令
    // while (cmd.start == 0)
    // {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }

    // settimeofday(&time_unix, NULL); // 收到指令后校准时间

    // while (1)
    // {
    //     uart_write_bytes(UART_NUM_1, "hello", 5);
    //     printf("%d,%d", cmd.start, cmd.unix_time);
    //     if (cmd.start == 1)
    //     {
    //         ms5837_get_data(&depth_data[index], NULL);
    //         unix_time[index] = time(NULL);

    //         // 电机控制深度   待定
    //         // depth_pid(depth_data[index], 2.5);

    //         if (depth_data[index] >= 2.5 && reached_time == 0) // 到达定深
    //         {
    //             reached_time = time(NULL);
    //         }
    //         if (time(NULL) - reached_time >= 45 && reached_time != 0)
    //         {
    //             stepper_move(100); // 上浮 步数待定
    //         }
    //         if (reached_time != 0 && depth_data[index] <= 0.1)
    //         {
    //             report_flag = 1;
    //             cmd.start = 0;
    //             reached_time = 0;
    //             break;
    //         }
    //         index++;
    //     }
    //     if (report_flag == 1)
    //     {
    //         for (int i = 0; i < index; i++)
    //         {
    //             print_time(unix_time[i]);
    //             printf("%f\n", depth_data[i]);
    //         }
    //         report_flag = 0;
    //     }
    //     vTaskDelay(10 / portTICK_PERIOD_MS);
    // }
}
