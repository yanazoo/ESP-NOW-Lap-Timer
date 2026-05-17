#pragma once

extern bool sdPresent;
extern bool sdPollEnabled;

void sdInit();
void sdSendStatus();
void sdCheckHotplug(uint32_t now);
void sdBeginRace();
void sdWriteLap(int slotIdx, uint32_t lapMs, int lapCount);
void sdEndRace();
void sdBeginBackup();
void sdWriteBackupRow(const char* name, const char* yomi,
                      const char* mac, int enter, int exit_, int slot);
void sdEndBackup();
void sdHandleRestore();
void sdListFiles();
void sdReadFile(const char* path);
void sdDeleteFile(const char* path);
