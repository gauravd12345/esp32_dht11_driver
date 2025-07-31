## ESP-IDF Driver for DHT11 sensor
Custom ESP32 driver for the DHT11 sensor using the ESP-IDF framework. 
### How to use
Include dht11.h and dht11.c in your project directory where the ESP-IDF environment has been set up. Make sure to edit the CMakeLists.txt file by adding the file that contains your <b>app_main</b> function.
For example, to use the dht11_test.c file shown below, edit your CMakeLists.txt file like so ```SRCS "dht11_test.c" "dht11.c" ```. 
### dht11_test.c
```#include <stdio.h>
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
```
## ESP-32 DHT11 Web Server
The ESP32 has onboard Wifi and Bluetooth capabilities and can be used to host a custom webserver. Using the DHT11 driver, we can build a webpage that displays temperature and humidity.

To get started, refer to https://esp32tutorials.com/esp32-web-server-esp-idf/ to setup your <b>Kconfig.projbuild</b> file and configuration details. Alternatively, you can use the Kconfig.projbuild already provided in this repo but you will still need to change your config details. 

Once you have set up your config files, you will also need to enable ```ESP_HTTPS_SERVER``` component. In your ESP-IDF terminal, enter ```idf.py menuconfig```, navigate to ```Component config```, then look for ```ESP HTTPS server``` and enable it. 

Then, include dht11_web_server.c in your project directory, edit your CMakeLists.txt and enter ```idf.py build flash monitor```. If everything went well, you should see an IP address in the terminal which you should then copy and paste into a web browser to see your webpage. Congrats! Your webpage should look something like this:

<img width="1918" height="927" alt="image" src="https://github.com/user-attachments/assets/66f299cf-0f83-4857-b76b-94d6f6c8fdad" />


## Resources
<li><a href="https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf">DHT11 Datasheet</a>
<li><a href="https://github.com/espressif/esp-idf/blob/v5.5/examples/protocols/http_server/simple/main/main.c">Espressif Web Server Example</a>
<li><a href="https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html">ESP-IDF GPIO documentation</a>
<li><a href="https://esp32tutorials.com/esp32-web-server-esp-idf/">https://esp32tutorials.com/esp32-web-server-esp-idf/</a>



