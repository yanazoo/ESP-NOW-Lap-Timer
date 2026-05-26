#pragma once
#include <stdint.h>
#include "config.h"

struct PilotState {
    uint8_t  uid[6];
    bool     hasUid;
    char     name[32];
    float    emaRssi;
    int      rawRssi;
    bool     crossing;
    int      peakRssi;
    uint32_t peakTime;
    uint32_t lastPeakTime;
    uint32_t lastLapTime;
    int      lapCount;
    int      entryThreshold;
    int      exitThreshold;
    uint32_t lastPacketTime;
};

extern PilotState pilots[MAX_PILOTS];

void initPilots();
void resetPilots();
int  findPilotByUID(const uint8_t* uid);
bool anyPilotRegistered();
void macToStr(const uint8_t* uid, char* buf);
void reportScanMac(const uint8_t* uid, int8_t rssi);
void resetScanTimers();
