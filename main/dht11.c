#include <stdio.h>
#include "dht11.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

/* Initalizing the DHT11 sensor */
void dht11_init(int gpio_pin){

    /* Creating START condition */
    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT); // ESP32 controls GPIO4 because it is set to OUTPUT_MODE
    gpio_set_level(gpio_pin, 0);
    esp_rom_delay_us(18000); // 18ms

    gpio_set_level(gpio_pin, 1);
    esp_rom_delay_us(40);

    /* Preparing DHT11 for data transmission */
    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT); // DHT11 controls GPIO4 because it is set to INPUT_MODE
    /*gpio_set_level(gpio_pin, 0);                    This is commented out because the DHT11 controls the pin automatically 
                                                      but this is essentially what it is doing
    */
    esp_rom_delay_us(80);

    //gpio_set_level(gpio_pin, 1);
    esp_rom_delay_us(80);

}


/* Reading data from the DHT11 sensor */
void dht11_read(int gpio_pin, uint8_t* temperature, uint8_t* humidity){
    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT);
    
    uint8_t data[5] = {0};
    int timeout = 100; // Timeout 
    for(int i = 0; i < 5; i++){

        // Writing a buffer to read the incoming stream of bits
        uint8_t buf = 0;
        for(int i = 0; i < 8; i++){
            /* Low voltage level that lasts 50µs */
            while(gpio_get_level(gpio_pin) == 0 && timeout--);

            /* Reading the bit */
            int timer = 0;
            while(gpio_get_level(gpio_pin) == 1 && timeout--){ // Checking how long VCC is held high
                esp_rom_delay_us(1);
                timer++;

            }

            int limit = 50;

            buf <<= 1;
            buf |= (timer > limit ? 1 : 0);

        }
        
        data[i] = buf;
        
    }
    esp_rom_delay_us(50);

    *temperature = data[2];
    *humidity = data[0];

}