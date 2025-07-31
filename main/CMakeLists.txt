idf_component_register(SRCS "dht11_web_server.c" "dht11.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio esp_timer nvs_flash esp_netif spi_flash esp_https_server esp_wifi)
