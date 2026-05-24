# ESP-NOW Lap Timer

🌐 [日本語](README.md) | **English**

A lap timer for **RC cars and FPV drones** built on ESP-NOW communication.
**No personal transponder required** — a low-cost, build-it-yourself lap-timing system anyone can put together.

> All you need is a tiny beacon on the vehicle (XIAO ESP32-C3, about $5–7 each) and a gate receiver at the trackside. For roughly **the price of a single commercial transponder**, you get a full system that times up to 4 vehicles at once.

**Highlights:**
- No modification to the vehicle (RC car / drone) — just mount a XIAO ESP32-C3/C6
- Low-cost setup with no personal transponder (the on-vehicle beacon is under ~$7)
- Roster of up to 20 pilots/machines, up to 4 vehicles timed simultaneously
- Gate crossings detected via RSSI peak detection + a RotorHazard-style state machine
- Smooth RSSI processing with an EMA filter (α = 0.3)
- Switchable **HS mode** / **Immediate (timing) mode**
- GitHub Dark themed Web UI (Japanese TTS, Canvas waveform graph, SD file browser)
- Automatic race CSV logging to SD card, plus roster backup/restore
- SD card hot-plug detection (insertion/removal detected dynamically)
- CSV saved with a UTF-8 BOM, so non-ASCII names are not garbled

---

## 🏎️ Lap Timing for RC Cars — A Low-Cost, Transponder-Free System

The standard way to time laps at an RC track is a "personal transponder + loop coil (timing line)" setup. But transponders run roughly $70–150 each, and a full trackside decoder is in the hundreds-to-thousands of dollars — the cost of entry has always been the barrier.

This system measures laps with just a **trackside gate receiver** and a **tiny beacon mounted on the vehicle**. The beacon broadcasts ESP-NOW packets, and the gate receiver detects the peak of their RSSI (signal strength) to decide when the vehicle crosses the timing line. No buried track loop and no expensive personal transponder needed.

### Cost Comparison (reference)

| | Personal transponder system | This system |
|---|---|---|
| Vehicle side | Transponder ~$70–150 each | XIAO ESP32-C3 **~$5–7 each** |
| Timing line side | Decoder + loop, hundreds–thousands of $ | ESP32 gate set **~$20–35** |
| Simultaneous timing | Varies by product | **Up to 4 vehicles** (20 in the roster) |
| Logging | Proprietary software / cloud | Auto CSV to SD card (opens in Excel) |
| Voice announce | Varies by product | Japanese TTS reads out lap count & lap time |

> Prices are typical reference figures as of 2026. For roughly **the price of one or two commercial transponders**, you can build the entire system, timing line included.

### Why It's Great for RC Cars

- **No vehicle modification**: just mount a thumb-sized, few-gram module. Power it from a power bank, the receiver BEC, etc.
- **Any category**: touring, buggy, drift, crawler — if you can fit the beacon, you can time it
- **Ideal for club runs and practice**: 4 vehicles at once, a 20-entry roster, automatic practice-lap timing and best-lap notifications
- **Indoor or outdoor**: tune the gate antenna height and directivity to suit the track
- **Accurate crossing detection**: RSSI peak detection pins the lap to the instant the vehicle is closest to the timing line

### How It Works (RC car case)

```
  [XIAO ESP32-C3 on the vehicle]   ... continuously broadcasts an ESP-NOW beacon
                │ RF (RSSI)
                ▼
  [Trackside gate receiver]        ... RSSI peak = timing-line crossing
                │ UART
                ▼
  [Web Node + phone/PC]            ... lap display, voice announce, CSV logging
```

> Drones work the same way. Mount the same beacon module on the aircraft and place the gate at the course gate position to time FPV drone-race laps.

---

## Hardware Architecture

```
┌─────────────────────────┐    UART (bidirectional)    ┌─────────────────────────┐
│   ESP32-WROVER-E-A      │ ←────────────────────────→ │   XIAO ESP32-S3-B       │
│  LilyGo TTGO T8 V1.8    │                            │     (Web Node)          │
│     (Gate Node)         │                            │                         │
│                         │                            │ - WiFi AP               │
│ - WiFi NULL mode        │                            │   SSID: ESP-NOW-LT      │
│ - Promiscuous mode      │                            │   PASS: esp-now-lt      │
│   ESP-NOW packet RX     │                            │   IP:   20.0.0.1        │
│ - EMA filtering         │                            │ - ESPAsyncWebServer     │
│ - RSSI state machine    │                            │ - WebSocket /ws         │
│ - SD card logging       │                            │ - LittleFS (Web UI)     │
│   CS=13 MOSI=15         │                            │ - NVS settings store    │
│   MISO=2 SCK=14         │                            │                         │
└─────────────────────────┘                            └─────────────────────────┘
   placed at the gate                                     placed in the pits / at hand
  antenna: patch recommended                              connect from phone over WiFi

┌─────────────────────────┐
│   XIAO ESP32-C3/C6      │
│   (Aircraft Node)       │
│   = on-vehicle beacon   │
│                         │
│ - ESP-NOW beacon TX     │
│ - mounted on RC car /   │
│   drone                 │
└─────────────────────────┘
```

> **Aircraft Node** is the name used in the code; in practice it is simply "the beacon you mount on the vehicle." The same module and the same firmware work for both RC cars and FPV drones.

### Pin Wiring

| ESP32-WROVER-E (Gate) | Direction | XIAO ESP32-S3 (Web) |
|-----------------------|-----------|---------------------|
| GPIO26 (TX1)          | →        | GPIO3 / D2 (RX1)    |
| GPIO25 (RX1)          | ←        | GPIO2 / D1 (TX1)    |
| GND                   | —        | GND                 |

---

## Lap Modes

Switchable from the Global Settings tab.

### HS Mode (default)

```
Race start
  └─ 1st gate crossing → recorded as "HS (Hole Shot)"
       └─ 2nd onward    → recorded as "Lap 1", "Lap 2", ...
```

- Cumulative time accumulates **from the HS crossing** (the travel time from start to HS is excluded)
- HS itself is excluded from best-lap evaluation

### Immediate (Timing) Mode

```
Race start
  └─ 1st gate crossing → recorded as "Lap 1" (time from start)
       └─ 2nd onward    → recorded as "Lap 2", "Lap 3", ...
```

- Cumulative time accumulates **from race start**

---

## Pilot Model

- **Roster**: stores up to 20 pilots/machines in NVS (name, reading, vehicle MAC, RSSI thresholds)
- **Active slots**: pick up to 4 from the roster and assign them to the gate
- **Vehicle identification**: each vehicle is uniquely identified by the hardware MAC address of its XIAO ESP32-C3
- **Vehicle scan**: an unregistered vehicle appears automatically in the scan list when its ESP-NOW frame is received
  - Already-registered MACs are not shown in the scan list
  - Shown as "online" while RSSI is being received
  - Power-on order (`firstSeenAt`) is recorded and used for automatic channel assignment

---

## Source Layout

### Gate Node (`src/gate_node/`)

| File | Role |
|------|------|
| `config.h` | Pin definitions and timing constants (EMA_ALPHA, COOLDOWN_MS, etc.) |
| `pilots.h/cpp` | PilotState array; init/lookup/scan reporting |
| `promiscuous.h/cpp` | ISR callback, FreeRTOS queue, WiFi config |
| `sd_gate.h/cpp` | SD init, hot-plug detection, race CSV, backup/restore, file browser |
| `uart_gate.h/cpp` | `sendLap` / `sendRssi` / `processWebCmd` dispatch |
| `main.cpp` | `setup()` / `loop()`, EMA state machine |

### Web Node (`src/web_node/`)

| File | Role |
|------|------|
| `config.h` | UART definitions, pilot limit, WiFi AP settings |
| `data_model.h` | All struct definitions, extern declarations of globals |
| `nvs_store.h/cpp` | NVS read/write (owns `roster[]` and `prefs`) |
| `gate_comm.h/cpp` | Gate UART protocol, `processGateLine` (owns `rt[]` and `laps[]`) |
| `json_api.h/cpp` | `rosterJson` / `activeJson` / `lapsJson` / `scanJson`, `handleBody` |
| `ws_handler.h/cpp` | WebSocket, `wsText`, `onWsEvent` |
| `http_routes.h/cpp` | All `server.on()` route registrations |
| `main.cpp` | `setup()` / `loop()` |

### Frontend (`data/`)

| File | Role |
|------|------|
| `index.html` | HTML shell + CSS (no inline JS) |
| `js/globals.js` | Constants, slot state, format helpers, `switchTab` (controls SD poll on tab switch) |
| `js/audio.js` | Web Audio API, 4-layer synthesized `beep` / `sfx` objects, TTS queue, `buildSpeech` (per lap mode) |
| `js/race.js` | Race cards, timer, race control, `applyActiveToSlots` |
| `js/config.js` | Roster CRUD, scan, auto channel assignment, SD backup/restore |
| `js/calib.js` | Canvas chart, rAF loop, threshold sliders, `syncCalibSliders` |
| `js/sd.js` | SD file browser (list, download, delete) |
| `js/ws.js` | WebSocket, `onMsg` dispatch, `loadRoster`, `loadAll`, app init |

---

## UART Protocol

### Gate → Web

```json
{"type":"lap",            "pilot":0,"uid":"AA:BB:CC:DD:EE:FF","rssi":-72,"ts":123456,"lapMs":42100}
{"type":"rssi",           "pilot":0,"rssi":-85,"raw":-87,"crossing":false,"signal":true,"ts":123460}
{"type":"ready",          "pilots":4}
{"type":"race_start_ack", "ts":123000}
{"type":"sd_status",      "present":true}
{"type":"scan",           "mac":"AA:BB:CC:DD:EE:FF","rssi":-75,"ts":123470}
{"type":"sd_pilot_row",   "name":"...","yomi":"...","mac":"...","enter":-80,"exit":-90}
{"type":"sd_restore_done"}
{"type":"sd_file_list",   "files":[{"name":"race_001.csv","size":1024}]}
{"type":"sd_file_line",   "path":"/race_001.csv","line":"0,Hayate,..."}
{"type":"sd_file_done",   "path":"/race_001.csv"}
{"type":"sd_delete_result","path":"/race_001.csv","ok":true}
```

### Web → Gate

```json
{"type":"cmd","action":"race_start"}
{"type":"cmd","action":"set_pilot",    "pilot":0,"uid":"AA:BB:CC:DD:EE:FF","name":"Hayate"}
{"type":"cmd","action":"set_threshold","pilot":0,"enter":-80,"exit":-90}
{"type":"cmd","action":"set_cooldown", "ms":3000}
{"type":"cmd","action":"scan_refresh"}
{"type":"cmd","action":"sd_poll",      "enable":true}
{"type":"cmd","action":"sd_begin_backup"}
{"type":"cmd","action":"sd_backup_row","name":"...","yomi":"...","mac":"...","enter":-80,"exit":-90}
{"type":"cmd","action":"sd_end_backup"}
{"type":"cmd","action":"sd_restore_request"}
{"type":"cmd","action":"sd_list_files"}
{"type":"cmd","action":"sd_read_file", "path":"/race_001.csv"}
{"type":"cmd","action":"sd_delete_file","path":"/race_001.csv"}
```

---

## RSSI Peak Detection Algorithm

```
raw RSSI → EMA filter (α=0.3, applied every loop) → Enter/Exit state machine → gate crossing event
```

### State Machine (RotorHazard-style)

```
CLEAR (idle)
 └─(ema > EnterAt)→ CROSSING
      ├─(ema > peak) → update peak, record peak time
      └─(ema < ExitAt and cooldown elapsed)→ sendLap(peakTime) → CLEAR
```

### Tunable Parameters

| Parameter        | Default  | Description                                         |
|------------------|----------|-----------------------------------------------------|
| EnterAt          | -80 dBm  | RSSI threshold to start a crossing                  |
| ExitAt           | -90 dBm  | RSSI threshold to end a crossing                    |
| EMA_ALPHA        | 0.3      | Smoothing coefficient                               |
| COOLDOWN_MS      | 3000 ms  | Minimum lap interval (behavior varies by lap mode)  |
| RSSI_INTERVAL_MS | 50 ms    | Telemetry interval (20 Hz)                          |

These are **per-pilot and adjustable at runtime** via the sliders on the Calib tab (applied to the Gate Node immediately).

---

## Web UI

**Connect:** WiFi SSID `ESP-NOW-LT` (PASS: `esp-now-lt`) → open `http://20.0.0.1` in a browser.

Notifications appear in the **header status bar**, not as bottom-of-screen popups.

### Race Tab

- 3-second countdown + race timer (start / stop / clear)
- Double-start prevention (buttons disabled during countdown)
- **Pause/Resume**: Stop = pause. Pressing Start again resumes from that point with no countdown (lap data retained). Time spent paused is excluded from both the timer display and lap times
- **Clear is state-aware**: disabled during a race, enabled after a stop. Clear saves results and resets everything = ready for the next race
- **Timer restore**: even after a page reload, a running/paused timer is restored from the correct elapsed time
- 4-column pilot grid: CROSSING badge, RSSI bar, best-lap + delta display
- Per-pilot lap table
  - **HS mode**: "HS" → "Lap 1", "Lap 2", ... / cumulative accumulates from the HS crossing
  - **Immediate mode**: "Lap 1", "Lap 2", ... / cumulative accumulates from start

### Config Tab

- **Vehicle scan**: auto-detected after power-on; only unregistered vehicles are listed
  - "Online" badge while RSSI is being received
  - 🤖 **Auto channel assignment**: assign Ch1–4 by power-on order (`firstSeenAt`)
  - ✖ **Clear all channels** button
  - Scan refresh button (manual gives feedback; auto-refresh runs silently every 5 s)
- **Pilots/machines**: up to 20 (name, reading, vehicle MAC, channel assignment)
  - Online entries are shown at the top
- **Global Settings**
  - Announce mode (default: name + lap + lap time)
  - Speech rate
  - Lap mode (HS mode / Immediate mode)
  - Cooldown time (in seconds)
  - Settings are saved automatically to `localStorage`
- **SD card**: shown only when an SD card is detected

### Calib Tab

- Per-pilot Canvas RSSI waveform graph (60 fps rAF loop, dynamic Y scale)
- Enter/Exit threshold sliders (auto-saved after an 800 ms debounce)

### SD Tab

- Lists files on the SD card
- Download (streamed over WebSocket) and delete race CSV files
- Downloaded CSV has a UTF-8 BOM (non-ASCII names are not garbled in Excel)

---

## SD Card Hot-Plug Detection

Polling runs only while the Config tab or SD tab is open. No SD operations are performed during a race or calibration.

| State                       | Check interval | Action                                              |
|-----------------------------|----------------|-----------------------------------------------------|
| No card (waiting for insert)| 500 ms         | Try `SD.end()` → `SD.begin()`                       |
| Card present (watch removal)| 3000 ms        | Try `SD.end()` → `SD.begin()` (only when idle)      |

On tab switch the browser POSTs to `/api/sd/poll`, toggling the gate_node `sdPollEnabled` flag on/off.

---

## CSV File Format

### Race CSV (`/race_NNN.csv`)

```
Slot,Name,UID,Lap,LapTime_ms,RSSI_dBm,Timestamp_ms
0,Hayate,AA:BB:CC:DD:EE:FF,1,42135,-75,123456
```

- Starts with a UTF-8 BOM → opens in Excel without garbled text

#### Converting lap times to a readable format in Excel

The `LapTime_ms` column is an integer in milliseconds (e.g. `12078` = 12.078 s).
**Keep the `LapTime_ms` column format as "General".** Applying a time format directly will not display correctly.

**Recommended steps (mm:ss.mmm display):**

1. In an empty column next to `LapTime_ms` (column E), e.g. column H, enter the header `LapTime`
2. In H2, enter the formula below and copy it down to the last row:
   ```
   =IF(E2=0,"HS",TEXT(E2/86400000,"m:ss.000"))
   ```
3. In HS mode the first lap (LapTime_ms=0) shows `HS`; everything else shows like `0:12.078`

| Desired format | Formula (E = LapTime_ms) | Cell format |
|---|---|---|
| mm:ss.mmm with HS handling (recommended) | `=IF(E2=0,"HS",TEXT(E2/86400000,"m:ss.000"))` | General |
| mm:ss.mmm (numeric cell) | `=E2/86400000` | `[m]:ss.000` |
| seconds (decimal) | `=E2/1000` | `0.000` |

### Pilot Backup (`/pilots.csv`)

```
name,yomi,mac,enter,exit,slot
Hayate,hayate,AA:BB:CC:DD:EE:FF,-80,-90,0
```

- Starts with a UTF-8 BOM
- The `slot` column is the channel assignment (0–3, `-1` if unassigned). Active slots are also restored automatically on restore

---

## REST API (Web Node)

| Endpoint                    | Method                     | Description                                       |
|-----------------------------|----------------------------|---------------------------------------------------|
| `/api/pilots`               | GET/POST                   | Get roster / add or update (saved to NVS)         |
| `/api/pilots/delete`        | POST `{id}`                | Delete a pilot from the roster                    |
| `/api/active`               | GET/POST                   | Get / set active slots                            |
| `/api/calib`                | POST `{id,enter,exit}`     | Update RSSI thresholds (NVS save + push to Gate)  |
| `/api/race/start`           | POST                       | Start race (send race_start to Gate; reset laps)  |
| `/api/race/stop`            | POST                       | Pause race (record stop time)                     |
| `/api/race/resume`          | POST                       | Resume from pause (keep laps; add paused time to timestamps to exclude it) |
| `/api/race/save`            | POST                       | Save results to SD and clear. Returns `{saved,reason}` (clears without saving when no SD / logging off) |
| `/api/laps`                 | GET                        | Get lap history                                   |
| `/api/scan`                 | GET                        | List scanned unregistered MACs                    |
| `/api/scan/refresh`         | POST                       | Reset the scan timer (forwarded to Gate)          |
| `/api/scan/clear`           | POST                       | Clear the scan list                               |
| `/api/settings`             | POST `{lapMode,cooldownMs}`| Lap mode / cooldown settings                      |
| `/api/status`               | GET                        | System status (raceRunning, lapCount, etc.)       |
| `/api/sd/status`            | GET                        | SD card presence                                  |
| `/api/sd/poll`              | POST `{enable}`            | SD hot-plug polling on/off                        |
| `/api/sd/pilots/backup`     | POST                       | Save roster to SD card                            |
| `/api/sd/pilots/restore`    | POST                       | Restore roster from SD card                       |
| `/api/sd/files/list`        | POST                       | Get SD file list (over WS)                        |
| `/api/sd/files/download`    | POST `{path}`              | Download a file (over WS)                         |
| `/api/sd/files/delete`      | POST `{path}`              | Delete a file (over WS)                           |

---

## Build & Flash

### Requirements

- PlatformIO Core or PlatformIO IDE (VS Code extension)

### Build + flash

```bash
# Gate Node (ESP32-WROVER-E / LilyGo TTGO T8 V1.8)
pio run -e gate_node -t upload

# Web Node (XIAO ESP32-S3)
pio run -e web_node -t upload

# Web UI (LittleFS) — required after changing JS/HTML
pio run -e web_node -t uploadfs

# Aircraft Node (XIAO ESP32-C3) — on-vehicle beacon
pio run -e aircraft_node -t upload

# Aircraft Node C6 (XIAO ESP32-C6) — on-vehicle beacon
pio run -e aircraft_node_c6 -t upload
```

### About the SD card (LilyGo TTGO T8 V1.8)

| Pin  | GPIO |
|------|------|
| CS   | 13   |
| MOSI | 15   |
| MISO | 2    |
| SCK  | 14   |

Use a FAT32-formatted microSD card.

- Race CSVs are auto-saved as `/race_001.csv`, `/race_002.csv`, ...
- The pilot backup is overwritten to `/pilots.csv`

---

## On-Vehicle Beacon Setup (Aircraft Node)

Just flash `aircraft_node` / `aircraft_node_c6` firmware onto a XIAO ESP32-C3 or XIAO ESP32-C6 and **mount it on the RC car or drone**.
It broadcasts ESP-NOW beacons automatically, so no extra configuration is needed.

- **Power**: for an RC car, supply 5V/3.3V from the receiver BEC, a power bank, or a dedicated LiPo (match the board's spec)
- **Mounting**: thumb-sized and a few grams — fix it under the body or on the chassis with double-sided tape
- **Identification**: each module's hardware MAC address uniquely identifies the vehicle (even multiple identical boards are told apart)

---

## Announcements (TTS)

Configurable in the Global Settings tab. Default is "name + lap + lap time".

| Mode                                  | Example readout                                            |
|---------------------------------------|-----------------------------------------------------------|
| Name + lap + lap time (default)       | "Hayate, hole shot, 42.1" / "Hayate, lap 1, 40.5"         |
| Name + lap time                       | "Hayate, 42.1"                                            |
| Beep only                             | Sound effect only                                         |
| Off                                   | None                                                      |

- **HS mode**: first crossing is "hole shot", subsequent ones are "Lap 1", "Lap 2", ...
- **Immediate mode**: "Lap 1", "Lap 2", ... from the first crossing

> The built-in TTS phrases are Japanese; the readout examples above are translated for reference.

---

## Sound Effects

Each sound is a 4-layer synthesis (fundamental + detuned twin + sub-octave for depth + upper harmonic for clarity) passed through a low-pass filter and a click-free envelope for a deep, full tone.

| Event             | Pitch range          | Waveform |
|-------------------|----------------------|----------|
| Lap detected      | 600Hz → 900Hz        | triangle |
| Best lap          | 600/900Hz x3 alt.    | triangle |
| Countdown         | 392Hz x3 → 523Hz     | triangle |
| ENTER threshold   | 523Hz                | triangle |
| EXIT threshold    | 740Hz                | triangle |

---

## Related Repositories

- PhobosLT (existing lap timer): [yanazoo/PhobosLT_4ch](https://github.com/yanazoo/PhobosLT_4ch)
- RotorHazard: [RotorHazard/RotorHazard](https://github.com/RotorHazard/RotorHazard)
