# ThermalEdge AC — ESP32 Smart AC Controller

An ESP32-based IR controller for Coolix-protocol split ACs, with a self-hosted
web dashboard, temperature monitoring, scheduling, and an automatic
cooling-verification system that retransmits the IR "on" command if the room
doesn't actually cool down after power-on.

Reachable at `http://smartac.local` on your local network — no cloud, no app,
no external service required.

---

## Features

- **IR control (Coolix protocol)** — power on/off, temperature, mode
  (cool/dry/auto/heat/fan), fan speed, swing, turbo, sleep
- **Cooling verification** — after every power-on, the device tracks room
  temperature and automatically retransmits the "on" command if the room
  hasn't dropped by the required amount within a configurable interval
  (default: 1°C within 10 minutes). Fully non-blocking, cancelled on reboot.
- **Temperature-triggered auto mode** — turn the AC on automatically once
  room temperature crosses a threshold, and off again once it's cooled
  enough — independent of any fixed schedule
- **Time-based scheduling** — up to 10 start/stop schedules, each
  independently enabled/disabled, backed by NTP for accurate local time
- **DS18B20 temperature sensor** with a rolling history log, exposed via
  `/api/history` for graphing
- **Self-hosted HTTP dashboard** (PsychicHttp) served directly from the
  ESP32 — no external hosting, works entirely on your local network
  - Live status, temperature, and cooling-verification state
  - Schedule and auto-mode management
  - WiFi credential and dashboard login management from the UI
  - OTA firmware updates (`/api/ota`)
- **Cookie-based session authentication** with a configurable
  username/password (see Security Notice above)
- **AP fallback** — if the configured home WiFi is unreachable at boot, the
  device starts its own access point so you can reconfigure it without a
  cable

---

## Hardware

| Component | Notes |
|---|---|
| ESP32 DevKit V1 | Any standard ESP32 dev board |
| DS18B20 | Digital temperature sensor → **GPIO27** (4.7 kΩ pull-up to 3.3V) |
| IR LED (+ driver transistor) | Coolix-protocol IR blaster → **GPIO4** |

---

## Required Libraries

Install via **Sketch → Include Library → Manage Libraries**:

| Library | Notes |
|---|---|
| PsychicHttp | 3.1.0 — HTTP server (this project runs HTTP only, no TLS) |
| IRremoteESP8266 | provides `IRsend` + `ir_Coolix` |
| OneWire | for DS18B20 |
| DallasTemperature | for DS18B20 |
| ArduinoJson | config + API payloads |

Also uses ESP32 core built-ins: `WiFi.h`, `ESPmDNS.h`, `Update.h`,
`Preferences.h`, `time.h`.

---

## Board Settings (Arduino IDE → Tools menu)

| Setting | Value |
|---|---|
| Board | ESP32 Dev Module |
| Upload Speed | 921600 |

---

## First-Time Setup

### 1. Fill in `config.h` locally (never commit real values)

```cpp
#define WIFI_SSID      "your-home-wifi-name"
#define WIFI_PASSWORD  "your-home-wifi-password"

#define MDNS_HOSTNAME  "smartac"

#define AUTH_USERNAME  "choose-a-username"
#define AUTH_PASSWORD  "choose-a-strong-password"

#define AP_SSID      "SmartAC-Setup"
#define AP_PASSWORD  "choose-a-setup-password"   // min 8 chars for a valid AP
```

### 2. Confirm the GPIO pins match your wiring

```cpp
#define IR_LED_PIN     4    // ESP32 GPIO4  — IR LED
#define ONE_WIRE_BUS   27   // ESP32 GPIO27 — DS18B20 data
```

### 3. Set your timezone (optional)

```cpp
#define NTP_TIMEZONE   "IST-5:30"   // POSIX TZ string
```

### 4. Flash the sketch

### 5. First boot

- If `WIFI_SSID`/`WIFI_PASSWORD` in `config.h` are valid, the device joins
  your network and is reachable at `http://smartac.local` (or its DHCP IP,
  printed to Serial).
- If it can't connect within 60 seconds, it falls back to its own AP
  (`AP_SSID`/`AP_PASSWORD` from `config.h`) at `http://192.168.4.1`.
- Log in with the `AUTH_USERNAME`/`AUTH_PASSWORD` you set in `config.h`.
  These aren't printed to Serial — check `config.h` (or the credentials you
  update later from the dashboard's Settings page) if you forget them.

### 6. Change credentials from the dashboard (recommended)

Once logged in, update the WiFi and login credentials from **Settings** —
these are then stored in ESP32 `Preferences` (NVS) on the device itself, not
in the source code, and persist across reboots and reflashes as long as NVS
isn't erased.

---

## Dashboard & API Routes

| Route | Purpose |
|---|---|
| `/`, `/home`, `/dashboard` | Main dashboard UI |
| `/auto` | Scheduling & temperature-mode UI |
| `/temp` | Temperature history view |
| `/settings` | WiFi, login, and cooling-verification settings UI |
| `/api/status` | Current AC + sensor + cooling-verification state |
| `/api/poweron`, `/api/poweroff` | Power control (routed through cooling verification) |
| `/api/settemp`, `/api/tempup`, `/api/tempdown` | Temperature control |
| `/api/setmode`, `/api/setfan` | Mode / fan speed |
| `/api/swing`, `/api/turbo`, `/api/sleep` | Toggle features |
| `/api/auto/schedules` | Manage time-based schedules |
| `/api/auto/settings` | Manage temperature-triggered auto mode |
| `/api/cooling` | Cooling-verification config/status |
| `/api/history` | Temperature history (for graphing) |
| `/api/ntp` | NTP sync status |
| `/api/login`, `/api/logout` | Session auth |
| `/api/settings/wifi`, `/api/settings/credentials` | Update WiFi / login creds (stored in NVS) |
| `/api/ota` | OTA firmware update |

All routes except `/api/login` and the AP-mode setup flow require a valid
session cookie.

---

## How Cooling Verification Works

1. Triggered only on an OFF → ON power transition.
2. Records the room's baseline temperature once, at the moment of power-on.
3. Waits `intervalMs` (default 10 minutes).
4. Compares current temperature to the recorded baseline.
5. If the drop is smaller than `requiredDrop` (default 1.0°C), retransmits
   the AC "on" IR command and waits again — the original baseline is kept
   across all retries, not reset.
6. Stops once the required drop is achieved, or the AC is turned off.
7. State is not persisted — a reboot cancels any in-progress verification.

Fully non-blocking (no `delay()` calls in the state machine), safe to run
alongside scheduling and temperature-mode logic every loop iteration.

---

## Known Limitations

- HTTP only — no TLS. Intended for trusted local networks; don't expose this
  device directly to the internet without additional protection (e.g. a VPN
  or reverse proxy with its own TLS termination).
- IR is one-way: the device cannot read the AC's actual internal state, only
  infer it from what commands it has sent and the DS18B20 reading — a
  manually-operated remote control used alongside this system can desync the
  two.
- Cooling-verification and auto-mode state live in RAM only and reset on
  reboot; schedules and global AC settings are persisted separately.

---

## Version

ESP32 build — full feature set:

IR AC control (Coolix protocol): power, temperature, mode, fan, swing, turbo, sleep
DS18B20 room temperature sensing with rolling history
Time-based scheduling (up to 10 schedules)
Temperature-triggered automatic power on/off
Automatic post-power-on cooling verification with IR retransmit
Self-hosted web dashboard and REST API (PsychicHttp, HTTP-only — no TLS)
Cookie-based session authentication with in-UI credential management
WiFi credential management from the dashboard, stored in ESP32 Preferences (NVS)
AP fallback for setup/recovery when home WiFi is unreachable
OTA firmware updates
mDNS (smartac.local) for access without knowing the device's IP
