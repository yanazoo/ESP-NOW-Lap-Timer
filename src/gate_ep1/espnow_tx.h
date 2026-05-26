// espnow_tx.h - send RSSI reports from sniffer to Gate ESP32
#pragma once
#include "config.h"

// Init ESP-NOW (ESP8285) and register the Gate ESP32 as a peer.
bool espnowBegin();

// Send one RSSI report. Non-blocking; drops silently on failure.
void espnowSendRssi(const uint8_t uid[6], int8_t rssi, uint8_t lq, uint32_t ts);
