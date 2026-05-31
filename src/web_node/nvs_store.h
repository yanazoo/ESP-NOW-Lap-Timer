#pragma once

void loadRosterConfig();
void loadActiveConfig();
void saveRosterPilot(int i);
void saveRosterCount();
void saveActive();
void nvsSaveRangeFromIndex(int fromId);  // rewrite all pilots at fromId..rosterCount-1 + clear last

// Admin password (soft auth gating non-race tabs from accidental viewer clobbering).
extern String adminPassword;   // loaded by loadAdminPassword(); default "admin"
void loadAdminPassword();
void saveAdminPassword(const String& pw);
