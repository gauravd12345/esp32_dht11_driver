#include "dht11.h"
#include "esp_log.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "DHT11_DRIVER";

void dht11_init(dht11_data *dht, gpio_num_t gpio_pin){
    dht->pin = gpio_pin;
    dht->temperature = 0.0f;
    dht->humidity = 0.0f;

    gpio_reset_pin(gpio_pin);
    gpio_set_pull_mode(gpio_pin, GPIO_PULLUP_ONLY);              
    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT_OUTPUT_OD); 
    vTaskDelay(1000 / portTICK_PERIOD_MS);

}

int data_line_pull(gpio_num_t pin, int value, int time_limit){
    int ticks = 0;
    while (gpio_get_level(pin) == value){
        if(ticks++ >= time_limit){
            return -1; // make sure it returns false here
        }
        ets_delay_us(1);

    }

    return ticks;
}

/* Getting the DHT11 response after START condition */
int dht11_start_response(gpio_num_t pin){
    int check1 = data_line_pull(pin, 0, 80);
    ESP_LOGW(TAG, "DHT High-to-Low %d", check1);
    if(check1 < 0){
        return -1;

    }

    int check2 = data_line_pull(pin, 1, 80);
    ESP_LOGW(TAG, "DHT Low-to-High %d", check2);
    if(check2 < 0){
        return -1;

    }

    int check3 = data_line_pull(pin, 0, 80);
    ESP_LOGW(TAG, "DHT Low-to-High %d", check3);
    if(check3 < 0){
        return -1;
    }

    return 1;
}

/* Creating the START condition */
void dht11_start(gpio_num_t pin){    
    gpio_set_direction(pin, GPIO_MODE_OUTPUT); // ESP32 controls GPIO4 because it is set to OUTPUT_MODE
    gpio_set_level(pin, 0);
    ets_delay_us(18000); 
    gpio_set_level(pin, 1);
    ets_delay_us(40); // 40ms
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}


/* Reading data from the DHT11 sensor */
int dht11_read(dht11_data *dht){
    uint8_t data[5] = {0};

    // if (esp_timer_get_time() - last_read_time < 2000000) {
    //     return dht11_r;
    // }
    // last_read_time = esp_timer_get_time();
    
    dht11_start(dht->pin); 
    int check1 = data_line_pull(dht->pin, 0, 80);
    
    if(check1 < 0){
        ESP_LOGW(TAG, "DHT High-to-Low %d", check1);
        return -1;

    }

    int check2 = data_line_pull(dht->pin, 1, 80);
    
    if(check2 < 0){
        ESP_LOGW(TAG, "DHT Low-to-High %d", check2);
        return -1;

    }

    int check3 = data_line_pull(dht->pin, 0, 80);
    
    if(check3 < 0){
        ESP_LOGW(TAG, "DHT Low-to-High %d", check3);
        return -1;
    }

    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            // Wait for LOW (start of bit)
            if (data_line_pull(dht->pin, 0, 58) < 0) return -1;
            int duration = data_line_pull(dht->pin, 1, 74);
            if (duration < 0) return -1;

            // If HIGH lasted longer than ~28us, it's a 1
            data[i] |= (duration > 28) << (7 - j);
        }
    }
    // Checksum validation
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if ((checksum & 0xFF) != data[4]) {
        ESP_LOGW(TAG, "Checksum failed: expected %02X, got %02X", checksum, data[4]);
        return -1;
    }

    dht->humidity = data[0] + data[1] / 10.0f;
    dht->temperature = data[2] + data[3] / 10.0f;

    return 0;

}