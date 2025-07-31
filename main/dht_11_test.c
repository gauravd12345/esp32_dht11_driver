#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "dht11.h"

#define DHT_PIN GPIO_NUM_4
static const char *TAG = "APP";

void app_main(void) {
    dht11_data dht11;
    dht11_init(&dht11, DHT_PIN);

    while (1) {
        if(dht11_read(&dht11) == 0){
            int temp_c = dht11.temperature;
            int temp_f = (temp_c * 9 / 5) + 32;
            int hum = dht11.humidity;
            ESP_LOGI(TAG, "Temp: %d°C / %d°F, Humidity: %d%%", temp_c, temp_f, hum);

        }
        else{
            ESP_LOGI(TAG, "ERROR READING DHT11");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);  // 2s delay between reads
    }
}
