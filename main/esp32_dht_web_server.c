#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "dht11.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
// Pin #4
//#define GPIO_NUM_18 18

static const char *TAG = "DHT11_DRIVER";

void app_main(void) {
    uint8_t temperature = 0;
    uint8_t humidity = 0;
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_reset_pin(GPIO_NUM_15);

    while(1){
        dht11_start(GPIO_NUM_15);
        dht11_read(GPIO_NUM_15, &temperature, &humidity);

        int temp_f = (temperature * 9 / 5) + 32;
        ESP_LOGI(TAG, "Temp: %d°C / %d°F, Humidity: %d%%", temperature, temp_f, humidity);
        vTaskDelay(pdMS_TO_TICKS(2000)); // 2000ms delay
        
    }
    
}
