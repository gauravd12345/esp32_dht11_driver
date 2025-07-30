#include <stdio.h>
#include "dht11.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "esp_log.h"


static const char *TAG = "DHT11_DRIVER";

int data_line_pull(int gpio_pin, int value, int time_limit){
    int ticks = 0;
    while (gpio_get_level(gpio_pin) == value){
        if(ticks > time_limit){
            return 0; // make sure it returns false here
        }
        ets_delay_us(1);
        ticks++;
    }

    return ticks;
}

/* Getting the DHT11 response after START condition */
bool dht11_start_response(int gpio_pin){
    int low = data_line_pull(gpio_pin, 0, 80);
    ESP_LOGW(TAG, "DHT High-to-Low %d", low);

    int high = data_line_pull(gpio_pin, 1, 80);
    ESP_LOGW(TAG, "DHT Low-to-High %d", high);

    if(!low || !high){
        return false;

    }
    return true;
}

/* Creating the START conditino */
void dht11_start(int gpio_pin){
    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT); // ESP32 controls GPIO4 because it is set to OUTPUT_MODE
    gpio_set_level(gpio_pin, 0);
    ets_delay_us(20000); // 20ms

    gpio_set_level(gpio_pin, 1);
    ets_delay_us(40);
    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT);
}


/* Reading data from the DHT11 sensor */
void dht11_read(int gpio_pin, uint8_t* temperature, uint8_t* humidity){
    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_pin, GPIO_PULLUP_ONLY);

    if(!dht11_start_response(gpio_pin)){
        ESP_LOGW(TAG, "Failed to generate START Condition response");
    }

    uint8_t data[5] = {0};
    for(int i = 0; i < 5; i++){
        // Writing a buffer to read the incoming stream of bits
        uint8_t buf = 0;
        for(int j = 0; j < 8; j++){
            int low_tim = data_line_pull(gpio_pin, 0, 50);
            //ESP_LOGI(TAG, "LOW_LEVEL_TIM: %d", low_tim);
            //ets_delay_us(5);  
              
            /* Reading the bit */
            int bit_tim = data_line_pull(gpio_pin, 1, 100);
            //ESP_LOGI(TAG, "HIGH_LEVEL_TIM: %d", bit_tim);

            int limit = 50;
            buf <<= 1;
            if (bit_tim >= limit) {
                buf |= 1;
                //ESP_LOGI(TAG, "Buffer: %d Bit: %d Time: %d", buf, 1, timer);
            }
            else{
                //ESP_LOGI(TAG, "Buffer: %d Bit: %d Time: %d", buf, 0, timer);
            }
            
        }
        
        data[i] = buf;
        
    }
    *temperature = data[2];
    *humidity = data[0];
      
}