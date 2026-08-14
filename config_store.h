#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class ConfigStore {
public:

    // ── WiFi credentials ──────────────────────────────────────────────────
    static bool loadWiFiCreds(String& ssid, String& pass) {
        File f = LittleFS.open(F("/wifi.json"), "r");
        if (!f) return false;
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, f) != DeserializationError::Ok) {
            f.close();
            return false;
        }
        f.close();
        ssid = doc["ssid"] | "";
        pass = doc["pass"] | "";
        return ssid.length() > 0;
    }

    static void saveWiFiCreds(const String& ssid, const String& pass) {
        File f = LittleFS.open(F("/wifi.json"), "w");
        if (!f) {
            Serial.println(F("[ConfigStore] ERROR: cannot open /wifi.json for write"));
            return;
        }
        StaticJsonDocument<256> doc;
        doc["ssid"] = ssid;
        doc["pass"] = pass;
        serializeJson(doc, f);
        f.close();
        Serial.println(F("[ConfigStore] WiFi credentials saved"));
    }

    // ── Auth credentials ──────────────────────────────────────────────────
    static bool loadAuthCreds(String& user, String& pass) {
        File f = LittleFS.open(F("/auth.json"), "r");
        if (!f) return false;
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, f) != DeserializationError::Ok) {
            f.close();
            return false;
        }
        f.close();
        user = doc["user"] | "";
        pass = doc["pass"] | "";
        return user.length() > 0;
    }

    static void saveAuthCreds(const String& user, const String& pass) {
        File f = LittleFS.open(F("/auth.json"), "w");
        if (!f) {
            Serial.println(F("[ConfigStore] ERROR: cannot open /auth.json for write"));
            return;
        }
        StaticJsonDocument<256> doc;
        doc["user"] = user;
        doc["pass"] = pass;
        serializeJson(doc, f);
        f.close();
        Serial.println(F("[ConfigStore] Auth credentials saved"));
    }

    // ── Scheduler / TempMode settings ─────────────────────────────────────
    static bool loadSchedulerData(JsonDocument& doc) {
        File f = LittleFS.open(F("/sched.json"), "r");
        if (!f) return false;
        DeserializationError err = deserializeJson(doc, f);
        f.close();
        return err == DeserializationError::Ok;
    }

    static void saveSchedulerData(JsonDocument& doc) {
        File f = LittleFS.open(F("/sched.json"), "w");
        if (!f) {
            Serial.println(F("[ConfigStore] ERROR: cannot open /sched.json for write"));
            return;
        }
        serializeJson(doc, f);
        f.close();
    }

    // ── Factory reset — wipe all config files ─────────────────────────────
    static void factoryReset() {
        LittleFS.remove(F("/wifi.json"));
        LittleFS.remove(F("/auth.json"));
        LittleFS.remove(F("/sched.json"));
        Serial.println(F("[ConfigStore] Factory reset — all config files removed"));
    }
};
