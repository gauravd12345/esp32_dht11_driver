#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "dht11.h"

// Wi-Fi config from menuconfig
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define DHT_PIN GPIO_NUM_4

static const char *TAG = "espressif";
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

int g_temperature_c = 0;
int g_temperature_f = 0;
int g_humidity = 0;

httpd_handle_t server = NULL;

// HTML response
esp_err_t send_web_page(httpd_req_t *req)
{
    char page[1024];
    snprintf(page, sizeof(page),
        "<!DOCTYPE html><html><head>"
        "<meta charset=\"UTF-8\">"
        "<title>ESP32 DHT11 Sensor</title></head><body>"
        "<h2>ESP32 DHT11 Sensor</h2>"
        "<p><strong>Temperature:</strong> %d&deg;C / %d&deg;F</p>"
        "<p><strong>Humidity:</strong> %d%%</p>"
        "</body></html>",
        g_temperature_c, g_temperature_f, g_humidity);

    return httpd_resp_send(req, page, HTTPD_RESP_USE_STRLEN);
}

// URI handler
esp_err_t get_req_handler(httpd_req_t *req) {
    return send_web_page(req);
}

httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t local_server = NULL;

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_req_handler
    };

    if (httpd_start(&local_server, &config) == ESP_OK) {
        httpd_register_uri_handler(local_server, &uri_get);
    }

    return local_server;
}

// Wi-Fi event handler
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying connection...");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Failed to connect");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        if (server == NULL)
            server = setup_server();
    }
}

// Connect to Wi-Fi
void connect_wifi(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi started");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
        ESP_LOGI(TAG, "Connected to SSID:%s", EXAMPLE_ESP_WIFI_SSID);
    else if (bits & WIFI_FAIL_BIT)
        ESP_LOGE(TAG, "Failed to connect to SSID:%s", EXAMPLE_ESP_WIFI_SSID);

    vEventGroupDelete(s_wifi_event_group);
}

// DHT11 task
void dht11_task(void *pvParameters)
{
    dht11_data dht;
    dht11_init(&dht, DHT_PIN);

    while (1)
    {
        if (dht11_read(&dht) == 0)
        {
            g_temperature_c = dht.temperature;
            g_temperature_f = (dht.temperature * 9 / 5) + 32;
            g_humidity = dht.humidity;
            ESP_LOGI(TAG, "Temp: %d°C / %d°F, Hum: %d%%", g_temperature_c, g_temperature_f, g_humidity);
        }
        else
        {
            ESP_LOGW(TAG, "Failed to read from DHT11");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Main
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    connect_wifi();

    xTaskCreate(&dht11_task, "dht11_task", 2048, NULL, 5, NULL);

    ESP_LOGI(TAG, "System ready");
}
