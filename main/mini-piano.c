#include <stdio.h>
#include <math.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/dac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_server.h"
#include "piano_html.h"

#define TAG "MINI_PIANO"
#define SAMPLE_RATE 8000

// Notas musicais (frequências em Hz)
typedef enum {
    NOTE_C4 = 261,
    NOTE_D4 = 293,
    NOTE_E4 = 329,
    NOTE_F4 = 349,
    NOTE_G4 = 392,
    NOTE_A4 = 440,
    NOTE_B4 = 493
} note_t;

// Função para tocar uma nota
void play_tone(dac_channel_t channel, int frequency, int duration_ms) {
    int samples = (SAMPLE_RATE * duration_ms) / 1000;
    for (int i = 0; i < samples; i++) {
        float amplitude = 127 + 127 * sin(2 * M_PI * frequency * i / SAMPLE_RATE); // Gera onda senoidal
        dac_output_voltage(channel, (uint8_t)amplitude);
        vTaskDelay(pdMS_TO_TICKS(1000 / SAMPLE_RATE));
    }
    dac_output_voltage(channel, 0); // Para o som
}

// Manipulador HTTP para tocar notas
esp_err_t note_handler(httpd_req_t *req) {
    char*  buf;
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;

    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Query String: %s", buf);
            if (strstr(buf, "note=C4")) play_tone(DAC_CHANNEL_1, NOTE_C4, 500);
            if (strstr(buf, "note=D4")) play_tone(DAC_CHANNEL_1, NOTE_D4, 500);
            if (strstr(buf, "note=E4")) play_tone(DAC_CHANNEL_1, NOTE_E4, 500);
            if (strstr(buf, "note=F4")) play_tone(DAC_CHANNEL_1, NOTE_F4, 500);
            if (strstr(buf, "note=G4")) play_tone(DAC_CHANNEL_1, NOTE_G4, 500);
            if (strstr(buf, "note=A4")) play_tone(DAC_CHANNEL_1, NOTE_A4, 500);
            if (strstr(buf, "note=B4")) play_tone(DAC_CHANNEL_1, NOTE_B4, 500);
        }
        free(buf);
    }
    httpd_resp_send(req, "Note played!", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t html_handler(httpd_req_t *req) {
    ESP_LOGI("HTTP", "Servindo página HTML");
    httpd_resp_send(req, piano_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Configuração do servidor HTTP
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        
        httpd_uri_t note_uri = {
            .uri       = "/play",
            .method    = HTTP_GET,
            .handler   = note_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &note_uri);

        httpd_uri_t html_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = html_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &html_uri);
    }
    return server;
}

// Configuração do Wi-Fi
void wifi_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32_Piano",
            .ssid_len = strlen("ESP32_Piano"),
            .channel = 1,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
}

void app_main(void) {
    // Inicialização do sistema
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(TAG, "ESP32 Mini Piano starting...");

    // Inicialização do Wi-Fi e servidor
    wifi_init();
    start_webserver();

    // Configuração do DAC
    dac_output_enable(DAC_CHANNEL_1);
}
