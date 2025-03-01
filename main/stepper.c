#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define A1 27
#define A2 14
#define B1 12
#define B2 13

#define pwm1 4
#define pwm2 5

#define timer 0

void stepper_init()
{
    gpio_config_t io_conf;
    io_conf.intr_type = 0;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << A1) | (1ULL << A2) | (1ULL << B1) | (1ULL << B2);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    ledc_timer_config_t timer_conf;
    timer_conf.duty_resolution = LEDC_TIMER_10_BIT;
    timer_conf.freq_hz = 100;
    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_conf.timer_num = timer;
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ledc_conf;
    ledc_conf.channel = LEDC_CHANNEL_0;
    ledc_conf.duty = 0;
    ledc_conf.gpio_num = pwm1;
    ledc_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_conf.timer_sel = LEDC_TIMER_0;
    ledc_channel_config(&ledc_conf);

    ledc_conf.channel = LEDC_CHANNEL_1;
    ledc_conf.gpio_num = pwm2;
    ledc_channel_config(&ledc_conf);
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

void pwm_set(int pwm_num, int pwm_duty)
{
    ledc_set_duty(LEDC_SPEED_MODE_MAX, pwm_num, pwm_duty);
    ledc_update_duty(LEDC_SPEED_MODE_MAX, pwm_num);
}

// steps正为正转，负为反转
// 50steps旋转一圈
void stepper_move(int steps)
{
    if (steps > 0)
    {
        for (int i = 0; i < steps; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                stepper_set(j);
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
    }
    else
    {
        steps = -steps;
        for (int i = 0; i < steps; i++)
        {
            for (int j = 3; j >= 0; j--)
            {
                stepper_set(j);
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
    }
}
