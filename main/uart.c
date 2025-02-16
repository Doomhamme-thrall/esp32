#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include "uart.h"
#include "esp_intr_alloc.h"

#define UART1_TX_PIN 17
#define UART1_RX_PIN 16
#define UART2_TX_PIN 4
#define UART2_RX_PIN 5

// 声明中断处理函数
static void IRAM_ATTR uart_intr_handle(void *arg);

void uart_init()
{
    const uart_port_t uart1_num = UART_NUM_1;
    uart_config_t uart1_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // 配置UART1参数
    ESP_ERROR_CHECK(uart_param_config(uart1_num, &uart1_config));
    // 设置UART1引脚
    ESP_ERROR_CHECK(uart_set_pin(uart1_num, UART1_TX_PIN, UART1_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // 安装UART1驱动
    ESP_ERROR_CHECK(uart_driver_install(uart1_num, 2048, 0, 0, NULL, 0));
    // 注册UART1中断处理函数
    ESP_ERROR_CHECK(uart_isr_register(uart1_num, uart_intr_handle, NULL, ESP_INTR_FLAG_IRAM, NULL));
    // 使能UART1接收中断
    ESP_ERROR_CHECK(uart_enable_rx_intr(uart1_num));
    ESP_LOGI("uart", "uart1 initialized");

    const uart_port_t uart2_num = UART_NUM_2;
    uart_config_t uart2_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // 配置UART2参数
    ESP_ERROR_CHECK(uart_param_config(uart2_num, &uart2_config));
    // 设置UART2引脚
    ESP_ERROR_CHECK(uart_set_pin(uart2_num, UART2_TX_PIN, UART2_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // 安装UART2驱动
    ESP_ERROR_CHECK(uart_driver_install(uart2_num, 2048, 0, 0, NULL, 0));
    ESP_LOGI("uart", "uart2 initialized");
}

// 将UART2接收的数据传入指定数组
void uart2_receive(char *data, size_t data_size)
{
    static size_t current_len = 0;
    while (1)
    {
        if (current_len >= data_size)
        {
            current_len = 0; // 缓冲区满，重置
        }

        // 从UART2读取数据
        int len = uart_read_bytes(UART_NUM_2, data + current_len, data_size - current_len, pdMS_TO_TICKS(100));
        if (len > 0)
        {
            current_len += len;
            // 查找换行符
            for (size_t i = 0; i < current_len; i++)
            {
                if (data[i] == '\n')
                {
                    data[i] = '\0';                                   // 替换换行符为字符串结束符
                    ESP_LOGI("UART2", "Received: %s", data);          // 打印接收到的数据
                    memmove(data, data + i + 1, current_len - i - 1); // 移动剩余数据到缓冲区开头
                    current_len -= (i + 1);                           // 更新缓冲区长度
                    break;
                }
            }
        }
        else
        {
            break; // 没有数据可读，退出循环
        }
    }
}

// 通过UART1发送数据
void uart1_report(const int *data, int size)
{
    uart_write_bytes(UART_NUM_1, (const char *)data, size);
}

// UART1中断处理函数
static void IRAM_ATTR uart_intr_handle(void *arg)
{
    uint16_t rx_fifo_len;
    uint8_t data[1024];
    int length = 0;

    // 获取UART1接收缓冲区中的数据长度
    uart_get_buffered_data_len(UART_NUM_1, (size_t *)&rx_fifo_len);
    // 读取数据
    length = uart_read_bytes(UART_NUM_1, data, rx_fifo_len, pdMS_TO_TICKS(100));

    // 处理接收到的数据
    if (length > 0)
    {
        // 在这里处理接收到的数据
        ESP_LOGI("UART1", "Received data: %.*s", length, data);
    }

    // 清除中断状态
    uart_clear_intr_status(UART_NUM_1, UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
}