#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "promiscuous.h"
#include "config.h"

QueueHandle_t packetQueue;

static void onEspNowRecv(const uint8_t* src, const uint8_t* data, int len) {
    if (len != (int)sizeof(GateEP1Packet_t)) return;
    GateEP1Packet_t pkt;
    memcpy(&pkt, data, sizeof(pkt));
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(packetQueue, &pkt, &woken);
    if (woken) portYIELD_FROM_ISR();
}

void setupEspNowGate() {
    packetQueue = xQueueCreate(64, sizeof(GateEP1Packet_t));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

    esp_now_init();
    esp_now_register_recv_cb(onEspNowRecv);

    Serial.printf("[Gate] ESP-NOW gate receiver on channel %d\n", ESPNOW_CHANNEL);
}
