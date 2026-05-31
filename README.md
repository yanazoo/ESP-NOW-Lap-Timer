# ESP-NOW Lap Timer

ESP-NOW通信を活用した、FPVドローンレース用ラップタイマー。

**特徴:**
- 機体側の改造不要（XIAO ESP32-C3/C6を機体に搭載するだけ）
- 最大20人のパイロット名簿（ロースター）管理、最大8スロット同時レース
- RSSIピーク検出 + RotorHazard準拠の状態機械でゲート通過を計測
- EMAフィルタ（α=0.3）による滑らかなRSSI処理
- **HSモード** / **計測モード** の切替に対応
- GitHub Darkテーマ Web UI（日本語TTS・Canvas波形グラフ・SDファイルブラウザ）
- SDカードへのレースCSV自動記録・パイロット情報バックアップ/復元
- SDカードのホットプラグ検知（挿入/抜去を動的に検知）
- CSV保存はUTF-8 BOM付きで日本語パイロット名も文字化けなし

---

## ハードウェア構成

```
┌─────────────────────────┐    UART (双方向)    ┌─────────────────────────┐
│   ESP32-WROVER-E-A      │ ←───────────────→  │   XIAO ESP32-S3-B       │
│  LilyGo TTGO T8 V1.8    │                   │     (Web Node)          │
│     (Gate Node)         │                   │                         │
│                         │                   │ - WiFi AP               │
│ - WiFi NULLモード        │                   │   SSID: ESP-NOW-LT      │
│ - Promiscuousモード       │                   │   PASS: esp-now-lt      │
│   ESP-NOW パケット受信   │                   │   IP:   20.0.0.1        │
│ - EMAフィルタ処理        │                   │ - ESPAsyncWebServer     │
│ - RSSI状態機械           │                   │ - WebSocket /ws         │
│ - SDカード記録           │                   │ - LittleFS (Web UI)     │
│   CS=13 MOSI=15         │                   │ - NVS設定保存           │
│   MISO=2 SCK=14         │                   │                         │
└─────────────────────────┘                   └─────────────────────────┘
       ゲートに設置                                   ピット・手元に設置
  アンテナ: パッチアンテナ推奨                       スマホからWiFi接続

┌─────────────────────────┐
│   XIAO ESP32-C3/C6      │
│   (Aircraft Node)       │
│                         │
│ - ESP-NOW ビーコン送信  │
│ - 機体に搭載            │
└─────────────────────────┘
```

### ピン配線

| ESP32-WROVER-E (Gate) | 方向 | XIAO ESP32-S3 (Web)  |
|-----------------------|------|----------------------|
| GPIO26 (TX1)          | →   | GPIO3 / D2 (RX1)     |
| GPIO25 (RX1)          | ←   | GPIO2 / D1 (TX1)     |
| GND                   | —   | GND                  |

---

## ラップモード

グローバル設定タブで切り替え可能。

### HSモード（デフォルト）

```
レーススタート
  └─ 1回目ゲート通過 → 「HS（ホールショット）」として記録
       └─ 2回目以降 → 「1周」「2周」... として記録
```

- 累計タイムは **HSゲート通過後** から積算（スタート〜HS間の移動時間は含まない）
- HS自体はベストラップ判定の対象外

### 計測モード（Immediate）

```
レーススタート
  └─ 1回目ゲート通過 → 「1周」として記録（スタートからの時間）
       └─ 2回目以降 → 「2周」「3周」... として記録
```

- 累計タイムは **レーススタート** から積算

---

## パイロットモデル

- **ロースター**: NVSに最大20人のパイロット情報（名前・読み方・機体MAC・RSSI閾値）を保存
- **アクティブスロット**: ロースターから最大8人を選択してゲートに割り当て
- **機体識別**: XIAO ESP32-C3のハードウェアMACアドレスでパイロットを一意識別
- **機体スキャン**: ESP-NOWフレームを受信すると未登録機体が自動スキャンリストに出現
  - 登録済みMACはスキャンリストに表示しない
  - RSSIを受信している間は「オンライン」と表示
  - 電源ON順（`firstSeenAt`）を記録し、自動チャンネル割当に利用

---

## ソースコード構成

### Gate Node (`src/gate_node/`)

| ファイル | 役割 |
|---------|------|
| `config.h` | ピン定義・タイミング定数（EMA_ALPHA、COOLDOWN_MSなど） |
| `pilots.h/cpp` | PilotState配列・初期化/検索/スキャン報告 |
| `promiscuous.h/cpp` | ISRコールバック・FreeRTOSキュー・WiFi設定 |
| `sd_gate.h/cpp` | SDカード初期化・ホットプラグ検知・レースCSV・バックアップ/復元・ファイルブラウザ |
| `uart_gate.h/cpp` | `sendLap`・`sendRssi`・`processWebCmd`ディスパッチ |
| `main.cpp` | `setup()`/`loop()`・EMA状態機械 |

### Web Node (`src/web_node/`)

| ファイル | 役割 |
|---------|------|
| `config.h` | UART定義・パイロット上限・WiFi AP設定 |
| `data_model.h` | 全構造体定義・グローバル変数extern宣言 |
| `nvs_store.h/cpp` | NVS読み書き（`roster[]`・`prefs`を所有） |
| `gate_comm.h/cpp` | Gate UARTプロトコル・`processGateLine`（`rt[]`・`laps[]`を所有） |
| `json_api.h/cpp` | `rosterJson`/`activeJson`/`lapsJson`/`scanJson`・`handleBody` |
| `ws_handler.h/cpp` | WebSocket・`wsText`・`onWsEvent` |
| `http_routes.h/cpp` | 全`server.on()`ルート登録 |
| `main.cpp` | `setup()`/`loop()` |

### Frontend (`data/`)

| ファイル | 役割 |
|---------|------|
| `index.html` | HTMLシェル＋CSS（インラインJSなし） |
| `js/globals.js` | 定数・スロット状態・フォーマット関数・`switchTab`（タブ切替時にSD Poll制御） |
| `js/audio.js` | Web Audio API・4層レイヤー合成の`beep`／`sfx`オブジェクト・TTSキュー・`buildSpeech`（ラップモード別） |
| `js/race.js` | レースカード・タイマー・レース制御・`applyActiveToSlots` |
| `js/config.js` | ロースターCRUD・スキャン・自動チャンネル割当・SDバックアップ/復元 |
| `js/calib.js` | Canvasチャート・rAFループ・閾値スライダー・`syncCalibSliders` |
| `js/sd.js` | SDファイルブラウザ（一覧・ダウンロード・削除） |
| `js/ws.js` | WebSocket・`onMsg`ディスパッチ・`loadRoster`・`loadAll`・アプリ初期化 |

---

## UART プロトコル

### Gate → Web

```json
{"type":"lap",            "pilot":0,"uid":"AA:BB:CC:DD:EE:FF","rssi":-72,"ts":123456,"lapMs":42100}
{"type":"rssi",           "pilot":0,"rssi":-85,"raw":-87,"crossing":false,"signal":true,"ts":123460}
{"type":"ready",          "pilots":8}
{"type":"race_start_ack", "ts":123000}
{"type":"sd_status",      "present":true}
{"type":"scan",           "mac":"AA:BB:CC:DD:EE:FF","rssi":-75,"ts":123470}
{"type":"sd_pilot_row",   "name":"...","yomi":"...","mac":"...","enter":-80,"exit":-90}
{"type":"sd_restore_done"}
{"type":"sd_file_list",   "files":[{"name":"race_001.csv","size":1024}]}
{"type":"sd_file_line",   "path":"/race_001.csv","line":"0,疾風翔,..."}
{"type":"sd_file_done",   "path":"/race_001.csv"}
{"type":"sd_delete_result","path":"/race_001.csv","ok":true}
```

### Web → Gate

```json
{"type":"cmd","action":"race_start"}
{"type":"cmd","action":"set_pilot",    "pilot":0,"uid":"AA:BB:CC:DD:EE:FF","name":"疾風翔"}
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

## RSSIピーク検出アルゴリズム

```
生RSSI → EMAフィルタ (α=0.3、毎ループ適用) → Enter/Exit 状態機械 → ゲート通過イベント
```

### 状態機械（RotorHazard準拠）

```
CLEAR（待機）
 └─(ema > EnterAt)→ CROSSING（通過中）
      ├─(ema > peak) → ピーク更新・ピーク時刻記録
      └─(ema < ExitAt かつ cooldown経過)→ sendLap(peakTime) → CLEAR
```

### 調整パラメータ

| パラメータ          | デフォルト | 説明                                         |
|---------------------|------------|----------------------------------------------|
| EnterAt             | -80 dBm    | 通過開始判定RSSI                             |
| ExitAt              | -90 dBm    | 通過終了判定RSSI                             |
| EMA_ALPHA           | 0.3        | 平滑化係数                                   |
| COOLDOWN_MS         | 3000 ms    | 最小ラップ間隔（ラップモードで挙動が変わる） |
| RSSI_INTERVAL_MS    | 50 ms      | テレメトリ送信間隔 (20Hz)                    |

キャリブタブのスライダーで**パイロット別・ランタイム変更可能**（Gate Nodeへ即時反映）。

---

## Web UI

**接続:** WiFi SSID `ESP-NOW-LT` (PASS: `esp-now-lt`) → ブラウザで `http://20.0.0.1`

通知はブラウザ下部のポップアップではなく**ヘッダーのステータスバー**に表示。

### Race タブ

- 3秒カウントダウン + レースタイマー（スタート/ストップ/クリア）
- ダブルスタート防止（カウントダウン中はボタン無効化）
- **一時停止/再開**: ストップ＝一時停止。再度スタートでカウントダウンなしにその時点から再開（周回データ保持）。停止中の経過時間はタイマー表示・ラップタイムの双方から除外
- **クリアの状態連動**: レース中はクリア無効、ストップ後に有効。クリアで結果保存＋全リセット＝次レース開始
- **タイマー復元**: ページをリロードしても走行中/一時停止中のタイマーが正しい経過時間から復元
- パイロットグリッド（最大8カード、4列×2段／モバイルは2列）：CROSSINGバッジ・RSSIバー・ベストラップ+デルタ表示
- パイロット別ラップ表
  - **HSモード**: 「HS」→「1周」「2周」... / 累計はHS通過後から積算
  - **計測モード**: 「1周」「2周」... / 累計はスタートから積算

### Config タブ

- **機体スキャン**: 電源ON後自動検出・未登録機体のみ一覧表示
  - RSSI受信中は「オンライン」バッジ表示
  - 🤖 **自動チャンネル割当**: 電源ON順（`firstSeenAt`）でCh1〜8を自動割当
  - ✖ **全チャンネル解除**ボタン
  - スキャン更新ボタン（手動のみフィードバック、自動更新は5秒ごと・静音）
- **パイロット情報**: 最大20人（名前・読み方・機体MAC・チャンネル割当）
  - オンライン中のパイロットを上位に表示
- **グローバル設定**
  - アナウンスモード（デフォルト: 名前＋周回＋ラップタイム）
  - 発話速度
  - ラップモード（HSモード / 計測モード）
  - クールダウン時間（秒単位）
  - 設定は `localStorage` に自動保存
- **SDカード**: SDカード検出時のみ表示

### Calib タブ

- パイロット別 Canvas RSSI波形グラフ（60fps rAFループ、動的Yスケール）
- Enter/Exit 閾値スライダー（変更から800msデバウンスで自動保存）

### 🔒 管理者ログイン（観覧 / 管理の分離）

複数端末から接続したときに、観覧目的の端末が誤ってレース設定を書き換えてしまうのを防ぐためのソフト認証。

- **観覧モード（既定）**：レースタブのラップ表示・タイマー・RSSIバーだけ閲覧可能。スタート/ストップ/クリアと、レース以外のタブは触れません
- **管理者モード**：設定・キャリブ・SDタブとレース制御ボタンをすべて操作可能
- **ログイン**：ヘッダー右端の「🔒 ログイン」ボタン、またはレース以外のタブをクリックするとパスワード入力モーダルが表示されます。ログイン後は同ボタンが「🔓 ログアウト」に変わります
- **パスワード変更**：設定タブの「🔒 管理者パスワード」欄に**表示されている値がそのままパスワード**です。書き換えて「保存」を押すだけ（確認欄なし）
- **セッション**：ブラウザを閉じると自動的にログアウトされます（`sessionStorage`保管）

> 設計趣旨は「誤操作からの保護」です。本格的なセキュリティではないので、悪意のある端末からのAPI直叩きは想定外です。レースイベントで観戦者と運営を分けたい用途に向いています。

### SD タブ

- SDカード内ファイル一覧表示
- レースCSVファイルのダウンロード（WebSocket経由ストリーミング）・削除
- ダウンロードCSVはUTF-8 BOM付き（Excelで日本語パイロット名が文字化けしない）

---

## SDカード ホットプラグ検知

設定タブまたはSDタブを開いているときのみポーリングを実行。レース中・キャリブ中はSD操作を行わない。

| 状態                   | チェック間隔 | 動作                                                       |
|------------------------|------------|------------------------------------------------------------|
| カードなし（挿入待ち） | 500 ms     | `SD.end()` → `SD.begin()` を試行                           |
| カードあり（抜去監視） | 3000 ms    | `SD.end()` → `SD.begin()` を試行（ファイル未使用時のみ）    |

タブ切替時にブラウザが `/api/sd/poll` へ POST し、gate_node の `sdPollEnabled` フラグを ON/OFF する。

---

## CSVファイル形式

### レースCSV (`/race_NNN.csv`)

```
Slot,Name,UID,Lap,LapTime_ms,RSSI_dBm,Timestamp_ms
0,疾風 翔,AA:BB:CC:DD:EE:FF,1,42135,-75,123456
```

- ファイル先頭にUTF-8 BOM付き → Excelで直接開いても文字化けしない

#### Excel でラップタイムを読みやすく変換する

`LapTime_ms` 列はミリ秒の整数（例: `12078` = 12.078秒）で保存されています。  
**LapTime_ms 列の書式は「標準」のままにしてください。** 直接時刻書式を当てると正しく表示されません。

**おすすめ手順（分:秒.ミリ秒表示）:**

1. `LapTime_ms` 列（E列）の隣の空き列（H列など）に見出し `LapTime` を入力
2. H2 に以下の数式を入力し、最終行までコピー:
   ```
   =IF(E2=0,"HS",TEXT(E2/86400000,"m:ss.000"))
   ```
3. HSモードの1周目（LapTime_ms=0）は `HS` と表示、それ以外は `0:12.078` のように表示される

| 表示したい形式 | 数式（E列が LapTime_ms の場合） | セル書式 |
|---|---|---|
| HS判定付き 分:秒.ミリ秒（推奨） | `=IF(E2=0,"HS",TEXT(E2/86400000,"m:ss.000"))` | 標準 |
| 分:秒.ミリ秒（数値セル） | `=E2/86400000` | `[m]:ss.000` |
| 秒（小数） | `=E2/1000` | `0.000` |

### パイロットバックアップ (`/pilots.csv`)

```
name,yomi,mac,enter,exit,slot
疾風 翔,はやてしょう,AA:BB:CC:DD:EE:FF,-80,-90,0
```

- ファイル先頭にUTF-8 BOM付き
- `slot` 列はチャンネル割当（0〜3、未割当は `-1`）。復元時にアクティブスロットも自動復元される

---

## REST API（Web Node）

| エンドポイント              | メソッド                   | 説明                                   |
|-----------------------------|----------------------------|----------------------------------------|
| `/api/pilots`               | GET/POST                   | ロースター取得・追加/更新（NVS保存）   |
| `/api/pilots/delete`        | POST `{id}`                | ロースターからパイロット削除           |
| `/api/active`               | GET/POST                   | アクティブスロット取得・設定           |
| `/api/calib`                | POST `{id,enter,exit}`     | RSSI閾値更新（NVS保存＋Gate反映）      |
| `/api/race/start`           | POST                       | レース開始（Gate Nodeにrace_start送信・周回リセット）|
| `/api/race/stop`            | POST                       | レース一時停止（停止時刻を記録）       |
| `/api/race/resume`          | POST                       | 一時停止から再開（周回保持・停止時間をタイムスタンプに加算して除外）|
| `/api/race/save`            | POST                       | レース結果をSDへ保存しクリア。`{saved,reason}` を返却（SDなし/ログOFF時は未保存でクリア）|
| `/api/laps`                 | GET                        | ラップ履歴取得                         |
| `/api/scan`                 | GET                        | スキャン済み未登録MAC一覧              |
| `/api/scan/refresh`         | POST                       | スキャンタイマーリセット（Gate転送）   |
| `/api/scan/clear`           | POST                       | スキャンリストクリア                   |
| `/api/settings`             | POST `{lapMode,cooldownMs}`| ラップモード・クールダウン設定         |
| `/api/status`               | GET                        | システム状態（raceRunning, lapCount等）|
| `/api/sd/status`            | GET                        | SDカード有無                           |
| `/api/sd/poll`              | POST `{enable}`            | SDホットプラグポーリング ON/OFF        |
| `/api/sd/pilots/backup`     | POST                       | ロースターをSDカードに保存             |
| `/api/sd/pilots/restore`    | POST                       | SDカードからロースター復元             |
| `/api/sd/files/list`        | POST                       | SDファイル一覧取得（WS経由）           |
| `/api/sd/files/download`    | POST `{path}`              | ファイルダウンロード（WS経由）         |
| `/api/sd/files/delete`      | POST `{path}`              | ファイル削除（WS経由）                 |

---

## ビルド・書き込み手順

### 必要環境

- PlatformIO Core または PlatformIO IDE (VS Code拡張)

### ビルド＋書き込み

```bash
# Gate Node (ESP32-WROVER-E / LilyGo TTGO T8 V1.8)
pio run -e gate_node -t upload

# Web Node (XIAO ESP32-S3)
pio run -e web_node -t upload

# Web UI (LittleFS) 書き込み — JS/HTML変更後に必要
pio run -e web_node -t uploadfs

# Aircraft Node (XIAO ESP32-C3) — 機体側
pio run -e aircraft_node -t upload

# Aircraft Node C6 (XIAO ESP32-C6) — 機体側
pio run -e aircraft_node_c6 -t upload
```

### SDカードについて（LilyGo TTGO T8 V1.8）

| ピン | GPIO |
|------|------|
| CS   | 13   |
| MOSI | 15   |
| MISO | 2    |
| SCK  | 14   |

FAT32フォーマットのmicroSDカードを使用。

- レースCSVは `/race_001.csv`, `/race_002.csv`, ... の形式で自動保存
- パイロットバックアップは `/pilots.csv` に上書き保存

---

## 機体側の設定（Aircraft Node）

XIAO ESP32-C3 または XIAO ESP32-C6 に `aircraft_node` / `aircraft_node_c6` ファームウェアを書き込み、機体に搭載するだけ。  
ESP-NOWビーコンを自動送信するため、追加設定は不要。

---

## アナウンス（TTS）

グローバル設定タブで変更可能。デフォルトは「名前＋周回＋ラップタイム」。

| モード                             | 読み上げ例                                                  |
|------------------------------------|-------------------------------------------------------------|
| 名前＋周回＋ラップタイム（デフォルト）| 「はやて、ホールショット、42秒1」「はやて、1周、40秒5」      |
| 名前＋ラップタイム                 | 「はやて、42秒1」                                           |
| ビープ音のみ                       | 効果音のみ                                                  |
| オフ                               | なし                                                        |

- **HSモード**: 1回目は「ホールショット」、2回目以降は「1周」「2周」...
- **計測モード**: 1回目から「1周」「2周」...

---

## 効果音

各音は4層レイヤー合成（基音＋デチューン双子＋サブオクターブ＝深み＋上倍音＝抜け）をローパスフィルタとクリックレスエンベロープに通し、深み・厚みのある音色にしている。

| イベント         | 音域                | 波形      |
|------------------|---------------------|-----------|
| ラップ検出       | 600Hz → 900Hz       | triangle  |
| ベストラップ     | 600/900Hz 交互×3    | triangle  |
| カウントダウン   | 392Hz×3 → 523Hz     | triangle  |
| ENTER閾値超過    | 523Hz               | triangle  |
| EXIT閾値下回り   | 740Hz               | triangle  |

---

## 関連リポジトリ

- PhobosLT（既存ラップタイマー）: [yanazoo/PhobosLT_4ch](https://github.com/yanazoo/PhobosLT_4ch)
- RotorHazard: [RotorHazard/RotorHazard](https://github.com/RotorHazard/RotorHazard)
