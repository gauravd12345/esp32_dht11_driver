#include "dht11.h"
#include "esp_log.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "DHT11_DRIVER";

/* Initalization function for DHT11 Sensor*/
void dht11_init(dht11_data *dht, gpio_num_t gpio_pin){
    dht->pin = gpio_pin;
    dht->temperature = 0.0f;
    dht->humidity = 0.0f;

    gpio_reset_pin(gpio_pin);
    gpio_set_pull_mode(gpio_pin, GPIO_PULLUP_ONLY);              
    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT_OUTPUT_OD); 
    vTaskDelay(1000 / portTICK_PERIOD_MS);

}

/* Checks how long DATA line is held at a certain value*/
int data_line_pull(gpio_num_t pin, int value, int time_limit){
    int ticks = 0;
    while (gpio_get_level(pin) == value){
        if(ticks++ >= time_limit){
            return -1;
        }
        ets_delay_us(1);

    }

    return ticks;
}

/* Getting the DHT11 response after START condition */
int dht11_start_response(gpio_num_t pin){
    // DATA line is pulled LOW for 80µs
    if(data_line_pull(pin, 0, 80) < 0){
        ESP_LOGW(TAG, "START RESPONSE #1: Pull to LOW voltage failed");
        return -1;

    }

    // DATA line is pulled HIGH for 80µs
    if(data_line_pull(pin, 1, 80) < 0){
        ESP_LOGW(TAG, "START RESPONSE #2: Pull to HIGH voltage failed");
        return -1;

    }
    
    // DATA line is pulled LOW for 50µs
    if(data_line_pull(pin, 0, 50) < 0){
        ESP_LOGW(TAG, "START RESPONSE #3: Pull to HIGH voltage failed");
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

    // Initiating the START condition
    dht11_start(dht->pin); 

    // Geting START condition response from DHT11
    if(dht11_start_response(dht->pin) < 0){
        return -1;
    }

    // At this point DHT response has been sent; now bits are going to be read
    for (int i = 0; i < 5; i++) {
        
        // Reading a byte at a time
        for (int j = 0; j < 8; j++) {

            // Waiting for the 50µs LOW voltage level
            if (data_line_pull(dht->pin, 0, 50) < 0){
                return -1;
            }

            // Getting the duration of the HIGH voltage level(Checking for max duration of 70µs)
            int duration = data_line_pull(dht->pin, 1, 70);

            if (duration < 0){
                return -1; // Error occured when reading bit
            }

            // If HIGH lasted longer than 28us, it's a 1
            data[i] |= (duration > 28) << (7 - j);
        }
    }


    // Checksum validation
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if ((checksum & 0xFF) != data[4]) {
        ESP_LOGW(TAG, "Checksum failed: expected %02X, got %02X", checksum, data[4]);
        return -1;
    }

    // Saving DHT11 sensor values
    dht->humidity = data[0] + data[1] / 10.0f;
    dht->temperature = data[2] + data[3] / 10.0f;

    return 0;
}