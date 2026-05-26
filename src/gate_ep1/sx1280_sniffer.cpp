// sx1280_sniffer.cpp - STUB. Implement against the EP1's SX1280 wiring.
//
// The SX1280 SPI pins are fixed by the EP1 PCB. Pull the exact GPIO mapping
// (NSS/SCK/MOSI/MISO/BUSY/DIO/RESET) from the ExpressLRS hardware target for
// HappyModel EP1 (targets/*.json or hardware/ folder in the ELRS repo).
//
// Easiest correct path: vendor the SX1280 driver from ExpressLRS
//   (src/lib/SX1280Driver) and call its init with the EP1 pin set, then expose
//   the four functions below. Re-implementing the driver from scratch is not
//   recommended.
//
// LoRa params MUST match the link being sniffed (BW/SF/CR per packet rate).
// For 250Hz 2.4GHz ELRS, copy the modem settings from the ELRS rate table.

#include "sx1280_sniffer.h"

bool sxBegin() {
    // TODO: init SPI, reset SX1280, read chip status/firmware reg, set LoRa modem.
    return false;
}

void sxSetFrequencyHz(uint32_t freqHz) {
    // TODO: write SetRfFrequency, then SetRx (continuous).
    (void)freqHz;
}

int8_t sxReadRssi() {
    // TODO: GetRssiInst or GetPacketStatus.rssiSync.
    return -127;
}

bool sxPacketReceived() {
    // TODO: check IRQ status RxDone, clear, return true once.
    return false;
}
