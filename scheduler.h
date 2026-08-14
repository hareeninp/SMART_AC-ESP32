#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>
#include "ac_controller.h"
#include "config.h"
#include "config_store.h"
#include "cooling_verification.h"

extern SensorManager sensorManager;

static constexpr uint8_t MAX_SCHEDULES = 10;

#define NTP_SERVER1  "pool.ntp.org"
#define NTP_SERVER2  "time.google.com"
#define NTP_TZ       NTP_TIMEZONE   // from config.h

struct Schedule {
    uint8_t startHH = 0;
    uint8_t startMM = 0;
    uint8_t stopHH  = 0;
    uint8_t stopMM  = 0;
    bool    enabled = false;
};

struct GlobalACSettings {
    uint8_t temp  = AC_DEFAULT_TEMP;
    uint8_t mode  = kCoolixCool;
    uint8_t fan   = kCoolixFanAuto;
    bool    swing = false;
    bool    turbo = false;
    bool    sleep = false;
};

struct TempModeSettings {
    bool    enabled     = false;
    uint8_t threshold   = 30;
    uint8_t coolingTemp = 25;
};

class SchedulerManager {
public:
    SchedulerManager() : _ntpSynced(false), _lastMinute(0xFF) {}

    void begin() {
        _loadAll();
        Serial.println(F("[Scheduler] Config loaded from LittleFS"));
        Serial.printf_P(PSTR("[Scheduler] %u schedule(s) restored\n"),
                        (unsigned)_schedCount);
    }

    void startNTP() {
        configTime(NTP_TZ, NTP_SERVER1, NTP_SERVER2);
        Serial.println(F("[NTP] Sync started"));
    }

    bool isNTPSynced() {
        if (_ntpSynced) return true;
        time_t now = time(nullptr);
        if (now > 1700000000UL) {
            _ntpSynced = true;
            struct tm t;
            localtime_r(&now, &t);
            Serial.printf_P(PSTR("[NTP] Synced – local time %02d:%02d:%02d\n"),
                            t.tm_hour, t.tm_min, t.tm_sec);
        }
        return _ntpSynced;
    }

    bool getLocalTime(uint8_t& hh, uint8_t& mm) {
        if (!isNTPSynced()) return false;
        time_t now = time(nullptr);
        struct tm t;
        localtime_r(&now, &t);
        hh = (uint8_t)t.tm_hour;
        mm = (uint8_t)t.tm_min;
        return true;
    }

    String getLocalTimeStr() {
        uint8_t hh, mm;
        if (!getLocalTime(hh, mm)) return F("--:--");
        char buf[6];
        snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)hh, (unsigned)mm);
        return String(buf);
    }

    void tick(ACController& ac, CoolingVerification& cv) {
        uint8_t hh, mm;
        if (!getLocalTime(hh, mm)) return;
        uint16_t curMin = (uint16_t)hh * 60u + mm;
        if (curMin == _lastMinute) return;
        _lastMinute = curMin;

        for (uint8_t i = 0; i < _schedCount; i++) {
            if (!_schedules[i].enabled) continue;
            const Schedule& s = _schedules[i];
            uint16_t startMin = (uint16_t)s.startHH * 60u + s.startMM;
            uint16_t stopMin  = (uint16_t)s.stopHH  * 60u + s.stopMM;
            bool isStart = (curMin == startMin);
            bool isStop  = (curMin == stopMin);
            if (isStart) {
                Serial.printf_P(PSTR("[Scheduler] Schedule %u START\n"), (unsigned)i);
                _applyGlobalSettings(ac, cv);
            }
            if (isStop && !isStart) {
                Serial.printf_P(PSTR("[Scheduler] Schedule %u STOP\n"), (unsigned)i);
                powerOffWithCV(ac, cv);
            }
        }
    }

    void tickTempMode(ACController& ac, float roomTemp, CoolingVerification& cv) {
        if (!_tempMode.enabled) return;
        if (roomTemp <= -998.0f) return;

        if (_coolingActive && !ac.getState().power) {
            Serial.println(F("[TempMode] AC turned off externally – re-arming"));
            _coolingActive = false;
        }

        if (!_coolingActive && roomTemp >= (float)_tempMode.threshold) {
            Serial.printf_P(PSTR("[TempMode] Trigger ON  room=%.1f >= thresh=%u\n"),
                            roomTemp, (unsigned)_tempMode.threshold);
            _applyGlobalSettings(ac, cv);
            ac.setTemp(_tempMode.coolingTemp);
            _coolingActive = true;
        } else if (_coolingActive && roomTemp <= (float)_tempMode.coolingTemp) {
            Serial.printf_P(PSTR("[TempMode] Trigger OFF room=%.1f <= cool=%u\n"),
                            roomTemp, (unsigned)_tempMode.coolingTemp);
            powerOffWithCV(ac, cv);
            _coolingActive = false;
        }
    }

    bool isCoolingActive() const { return _coolingActive; }

    const GlobalACSettings& getGlobalSettings() const { return _global; }

    bool setGlobalSettings(const GlobalACSettings& g) {
        if (g.temp < AC_TEMP_MIN || g.temp > AC_TEMP_MAX) return false;
        if (g.mode != kCoolixCool && g.mode != kCoolixDry &&
            g.mode != kCoolixAuto && g.mode != kCoolixHeat &&
            g.mode != kCoolixFan)  return false;
        if (g.fan != kCoolixFanAuto && g.fan != kCoolixFanMin &&
            g.fan != kCoolixFanMed  && g.fan != kCoolixFanMax) return false;
        _global = g;
        _saveAll();
        return true;
    }

    uint8_t getScheduleCount() const { return _schedCount; }

    const Schedule* getSchedule(uint8_t idx) const {
        if (idx >= _schedCount) return nullptr;
        return &_schedules[idx];
    }

    uint8_t addSchedule(const Schedule& s) {
        if (_schedCount >= MAX_SCHEDULES) return 0xFF;
        _schedules[_schedCount] = s;
        uint8_t newIdx = _schedCount++;
        _saveAll();
        Serial.printf_P(PSTR("[Scheduler] Added schedule %u\n"), (unsigned)newIdx);
        return newIdx;
    }

    bool updateSchedule(uint8_t idx, const Schedule& s) {
        if (idx >= _schedCount) return false;
        _schedules[idx] = s;
        _saveAll();
        Serial.printf_P(PSTR("[Scheduler] Updated schedule %u\n"), (unsigned)idx);
        return true;
    }

    bool deleteSchedule(uint8_t idx) {
        if (idx >= _schedCount) return false;
        for (uint8_t i = idx; i < _schedCount - 1; i++)
            _schedules[i] = _schedules[i + 1];
        _schedCount--;
        _saveAll();
        Serial.printf_P(PSTR("[Scheduler] Deleted schedule %u\n"), (unsigned)idx);
        return true;
    }

    bool setEnabled(uint8_t idx, bool en) {
        if (idx >= _schedCount) return false;
        _schedules[idx].enabled = en;
        _saveAll();
        return true;
    }

    const TempModeSettings& getTempMode() const { return _tempMode; }

    bool setTempMode(const TempModeSettings& t) {
        if (t.threshold < AC_TEMP_MIN || t.threshold > 50) return false;
        if (t.coolingTemp < AC_TEMP_MIN || t.coolingTemp > AC_TEMP_MAX) return false;
        _tempMode = t;
        _saveAll();
        return true;
    }

    void factoryReset() {
        LittleFS.remove(F("/sched.json"));
        _global     = GlobalACSettings{};
        _tempMode   = TempModeSettings{};
        _schedCount = 0;
        Serial.println(F("[Scheduler] Factory reset"));
    }

private:
    GlobalACSettings _global;
    Schedule         _schedules[MAX_SCHEDULES];
    uint8_t          _schedCount  = 0;
    bool             _ntpSynced;
    uint16_t         _lastMinute;
    TempModeSettings _tempMode;
    bool             _coolingActive = false;

    void _applyGlobalSettings(ACController& ac, CoolingVerification& cv) {
        powerOnWithCV(ac, sensorManager, cv);
        ac.setTemp(_global.temp);
        ac.setMode(_global.mode);
        ac.setFan(_global.fan);
        const ACState& st = ac.getState();
        if (_global.swing != st.swing) ac.toggleSwing();
        if (_global.turbo != st.turbo) ac.toggleTurbo();
        if (_global.sleep != st.sleep) ac.toggleSleep();
    }

    void _saveAll() {
        DynamicJsonDocument doc(2048);
        doc["gl_temp"]  = _global.temp;
        doc["gl_mode"]  = _global.mode;
        doc["gl_fan"]   = _global.fan;
        doc["gl_swing"] = _global.swing;
        doc["gl_turbo"] = _global.turbo;
        doc["gl_sleep"] = _global.sleep;
        doc["tm_en"]    = _tempMode.enabled;
        doc["tm_thr"]   = _tempMode.threshold;
        doc["tm_cool"]  = _tempMode.coolingTemp;
        doc["sched_count"] = _schedCount;
        JsonArray arr = doc.createNestedArray("schedules");
        for (uint8_t i = 0; i < _schedCount; i++) {
            JsonObject o = arr.createNestedObject();
            o["sHH"] = _schedules[i].startHH;
            o["sMM"] = _schedules[i].startMM;
            o["eHH"] = _schedules[i].stopHH;
            o["eMM"] = _schedules[i].stopMM;
            o["en"]  = _schedules[i].enabled;
        }
        ConfigStore::saveSchedulerData(doc);
    }

    void _loadAll() {
        DynamicJsonDocument doc(2048);
        if (!ConfigStore::loadSchedulerData(doc)) {
            Serial.println(F("[Scheduler] No saved config – using defaults"));
            return;
        }
        _global.temp  = doc["gl_temp"]  | (int)AC_DEFAULT_TEMP;
        _global.mode  = doc["gl_mode"]  | (int)kCoolixCool;
        _global.fan   = doc["gl_fan"]   | (int)kCoolixFanAuto;
        _global.swing = doc["gl_swing"] | false;
        _global.turbo = doc["gl_turbo"] | false;
        _global.sleep = doc["gl_sleep"] | false;
        _tempMode.enabled     = doc["tm_en"]   | false;
        _tempMode.threshold   = doc["tm_thr"]  | 30;
        _tempMode.coolingTemp = doc["tm_cool"] | 25;
        _schedCount = doc["sched_count"] | 0;
        if (_schedCount > MAX_SCHEDULES) _schedCount = MAX_SCHEDULES;
        JsonArray arr = doc["schedules"].as<JsonArray>();
        uint8_t i = 0;
        for (JsonObject o : arr) {
            if (i >= _schedCount) break;
            _schedules[i].startHH = o["sHH"] | 0;
            _schedules[i].startMM = o["sMM"] | 0;
            _schedules[i].stopHH  = o["eHH"] | 0;
            _schedules[i].stopMM  = o["eMM"] | 0;
            _schedules[i].enabled = o["en"]  | false;
            i++;
        }
    }
};
