#include <stddef.h>

typedef struct
{
    int unix_time;
    int start;
    int steps;
} commands;
extern commands cmd;

void uart_init();
void uart_event_task(void *pvParameters);
