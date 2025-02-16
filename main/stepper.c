#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define A1 0
#define A2 1
#define B1 2
#define B2 3

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
    timer_conf.freq_hz = 1000;
    timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    timer_conf.timer_num = timer;
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ledc_conf;
    ledc_conf.channel = LEDC_CHANNEL_0;
    ledc_conf.duty = 0;
    ledc_conf.gpio_num = pwm1;
    ledc_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
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
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, pwm_num, pwm_duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, pwm_num);
}

void stepper_forward(int step_num)
{
    for (int i = 0; i < (step_num) / 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            stepper_set(j);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    for (int i = 0; i < (step_num % 4); i++)
    {
        stepper_set(i);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void stepper_back(int step_num)
{
    for (int i = 0; i < (step_num) / 4; i++)
    {
        for (int j = 3; j >= 0; j--)
        {
            stepper_set(j);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    for (int i = (step_num % 4); i >= 0; i--)
    {
        stepper_set(i);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
