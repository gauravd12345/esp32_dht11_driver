idf_component_register(SRCS "main.c" "dht11.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio esp_timer)
