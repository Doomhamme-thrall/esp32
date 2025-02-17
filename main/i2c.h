#include <stddef.h>
#include <stdint.h>

void i2c_init();
void i2c_write(uint8_t addr, uint8_t *data_wr, size_t size);
void i2c_read(uint8_t addr, uint8_t *data_rd, size_t size);
