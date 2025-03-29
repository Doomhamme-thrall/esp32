#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_rom_sys.h"
#include "esp_task.h"
#include "esp_task_wdt.h"
#include "uart.h"

#define A1 27
#define A2 14
#define B1 12
#define B2 13

#define pwm1 4
#define pwm2 8

#define timer 0

#define max_lenth 4500

extern TaskHandle_t uart_task_handle;

void stepper_init()
{
    gpio_config_t io_conf;
    io_conf.intr_type = 0;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << A1) | (1ULL << A2) | (1ULL << B1) | (1ULL << B2);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void stepper_set(int phrase)
{
    switch (phrase)
    {
    case 0:
        gpio_set_level(A1, 1);
        gpio_set_level(A2, 0);
        gpio_set_level(B1, 0);
        gpio_set_level(B2, 1);
        break;
    case 1:
        gpio_set_level(A1, 0);
        gpio_set_level(A2, 1);
        gpio_set_level(B1, 0);
        gpio_set_level(B2, 1);
        break;
    case 2:
        gpio_set_level(A1, 0);
        gpio_set_level(A2, 1);
        gpio_set_level(B1, 1);
        gpio_set_level(B2, 0);
        break;
    case 3:
        gpio_set_level(A1, 1);
        gpio_set_level(A2, 0);
        gpio_set_level(B1, 1);
        gpio_set_level(B2, 0);
        break;

    default:
        break;
    }
}

// steps正为正转，负为反转
// 1steps=1.8deg
void stepper_move(int steps)
{
    if (steps > 0)
    {
        for (int i = 0; i < steps; i++)
        {
            stepper_set(i % 4);
            esp_rom_delay_us(5000);
            // vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    else
    {
        for (int i = 0; i < -steps; i++)
        {
            stepper_set((3 - (i % 4)));
            esp_rom_delay_us(5000);
            // vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

void stepper_position(int target_position, int *current_position)
{
    if (target_position > max_lenth)
    {
        target_position = max_lenth;
    }
    if (target_position < 0)
    {
        target_position = 0;
    }
    int steps = target_position - (*current_position);
    stepper_move(steps);
    *current_position = target_position;
}