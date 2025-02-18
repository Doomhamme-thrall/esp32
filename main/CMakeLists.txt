idf_component_register(SRCS "main.c" 
                            "uart.c" 
                            "stepper.c" 
                            "ms5837.c"
                            "get_time.c"
                            
                    PRIV_REQUIRES spi_flash driver
                    INCLUDE_DIRS "")
