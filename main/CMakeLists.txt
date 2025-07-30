idf_component_register(SRCS "esp32_dht_web_server.c" "dht11.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio)
