#include <time.h>
#include "esp_log.h"
#include "uart.h"
#include "driver/uart.h"

void get_time()
{
    // *time_unix = time(NULL);
}

void print_time(int time_unix)
{
    time_t now = time_unix;
    struct tm timeinfo;
    char buffer[64];

    setenv("TZ", "CST-8", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    printf("Current date and time: %s\n", buffer);
}

void time_sync(int *time_unix)
{
}