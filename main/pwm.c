#include "driver/ledc.h"

// 要输出PWM的引脚
#define PWM_PORT 1

// 初始化脉冲信号
void pwm_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0};

    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = PWM_PORT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .hpoint = 0,
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);
}

// 初始化方向信号