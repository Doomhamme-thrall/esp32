#include <stddef.h>

typedef struct
{
    int unix_time;
    int start;
} commands;

void uart_init();
void uart2_receive(char *data, size_t data_size);
