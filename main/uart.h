#include <stddef.h>

typedef struct
{
    int unix_time;
    int start;
} commands;
extern commands cmd;

void uart_init();
