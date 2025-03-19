#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include "uart.h"
#include "stepper.h"

#define UART1_TX_PIN 16
#define UART1_RX_PIN 17

#define DATA_QUEUE_SIZE 10
#define DATA_BUFFER_SIZE 1024

static const char *TAG = "uart_events";
static QueueHandle_t uart1_queue;
static QueueHandle_t data_queue;

commands cmd = {
    .unix_time = 0,
    .start = 0};

static void
uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t *dtmp = (uint8_t *)malloc(DATA_BUFFER_SIZE);
    for (;;)
    {
        // 等待 UART 事件
        if (xQueueReceive(uart1_queue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, DATA_BUFFER_SIZE);
            ESP_LOGI(TAG, "uart[%d] event:", UART_NUM_1);
            switch (event.type)
            {
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(UART_NUM_1, dtmp, event.size, portMAX_DELAY);
                ESP_LOGI(TAG, "[DATA EVT]: %s", dtmp);
                char *token = strtok((char *)dtmp, ",");
                if (token != NULL)
                {
                    cmd.unix_time = atoi(token);
                    token = strtok(NULL, ",");
                    if (token != NULL)
                    {
                        cmd.start = (int8_t)atoi(token);
                    }
                }
                printf("data processed");
                break;
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                uart_flush_input(UART_NUM_1);
                xQueueReset(uart1_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                uart_flush_input(UART_NUM_1);
                xQueueReset(uart1_queue);
                break;
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            case UART_PATTERN_DET:
                size_t buffered_size;
                uart_get_buffered_data_len(UART_NUM_1, &buffered_size);
                int pos = uart_pattern_pop_pos(UART_NUM_1);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1)
                {
                    uart_flush_input(UART_NUM_1);
                }
                else
                {
                    uart_read_bytes(UART_NUM_1, dtmp, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[3 + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(UART_NUM_1, pat, 3, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", dtmp);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void uart_init()
{
    const uart_port_t uart_num = UART_NUM_1;
    uart_config_t uart1_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(uart_num, &uart1_config);
    uart_set_pin(uart_num, UART1_TX_PIN, UART1_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, 2048, 2048, 20, &uart1_queue, 0);

    uart_enable_pattern_det_baud_intr(uart_num, '+', 3, 9, 0, 0);
    uart_pattern_queue_reset(UART_NUM_1, 20);

    // 创建处理 UART 事件的任务
    xTaskCreate(uart_event_task, "uart_event_task", 3072, NULL, 12, NULL);

    ESP_LOGI("uart", "uart1 initialized");
}
