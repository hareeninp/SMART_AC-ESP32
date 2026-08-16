#pragma once

// ── WiFi ──────────────────────────────────────────────────────────────────────
#define WIFI_SSID      
#define WIFI_PASSWORD  

// ── mDNS ─────────────────────────────────────────────────────────────────────
#define MDNS_HOSTNAME  

// ── Server ───────────────────────────────────────────────────────────────────
#define HTTP_PORT      80


// ── GPIO ─────────────────────────────────────────────────────────────────────
#define IR_LED_PIN     4           // IR LED (COOLIX)   – ESP32 GPIO4
#define ONE_WIRE_BUS   27          // DS18B20 data       – ESP32 GPIO27          

// ── Authentication ────────────────────────────────────────────────────────────
#define AUTH_USERNAME  
#define AUTH_PASSWORD  
#define SESSION_COOKIE "smartac_session"
#define SESSION_MAXAGE 86400        // 24 hours in seconds

// ── AC defaults ───────────────────────────────────────────────────────────────
#define AC_DEFAULT_TEMP  24
#define AC_TEMP_MIN      17
#define AC_TEMP_MAX      30

// ── AP Fallback (used when home WiFi is unavailable) ──────────────────────────
#define AP_SSID      
#define AP_PASSWORD  

// ── NTP / Timezone ────────────────────────────────────────────────────────────
#define NTP_TIMEZONE   "IST-5:30"
