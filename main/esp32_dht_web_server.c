#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "dht11.h"

// Pin #4
#define GPIO_NUM_4 (4)

static const char *TAG = "DHT11_DRIVER";

void app_main(void) {
    dht11_init(GPIO_NUM_4);

    uint8_t temperature = 0;
    uint8_t humidity = 0;

    while(1){
        dht11_read(GPIO_NUM_4, &temperature, &humidity);
        ESP_LOGI(TAG, "Temperature: %d°C, Humidity: %d%%", temperature, humidity);
        vTaskDelay(pdMS_TO_TICKS(2000)); // 2000ms delay

    }
    
}
