#pragma once

#include <Arduino.h>
#include "ac_controller.h"
#include "sensor.h"

// ── State machine states ───────────────────────────────────────────────────
enum class CVState : uint8_t {
    IDLE,       // not running
    WAITING,    // waiting for intervalMs to elapse
    CHECKING,   // interval elapsed, evaluating temperature drop
    VERIFIED,   // cooling confirmed
    RETRYING    // retransmitting IR, then back to WAITING
};

// ── CoolingVerification ────────────────────────────────────────────────────
class CoolingVerification {
public:
    // ── Public config fields (read/write from routes.h) ───────────────────
    bool          enabled      = true;
    unsigned long intervalMs   = 10UL * 60000UL;   // 10 minutes default
    float         requiredDrop = 1.0f;              // °C required drop

    // ── Public status fields (read from routes.h) ─────────────────────────
    bool     active       = false;
    bool     verified     = false;
    uint8_t  retryCount   = 0;
    float    baselineTemp = 0.0f;
    uint32_t baselineTime = 0;      // millis() when verification started

    // ── tick() — call every loop() iteration ──────────────────────────────
    void tick(SensorManager& sensor, ACController& ac) {
        if (!enabled || _state == CVState::IDLE) return;

        float    currentTemp = sensor.getTemp();
        uint32_t now         = millis();

        switch (_state) {

            case CVState::WAITING:
                if ((now - _waitStart) >= intervalMs) {
                    _state = CVState::CHECKING;
                    Serial.println(F("[CoolVerify] Interval elapsed — checking temp drop"));
                }
                break;

            case CVState::CHECKING: {
                if (!sensor.isValid()) {
                    Serial.println(F("[CoolVerify] Sensor invalid — retrying next interval"));
                    _waitStart = millis();
                    _state = CVState::WAITING;
                    break;
                }
                float drop = baselineTemp - currentTemp;
                Serial.printf_P(PSTR("[CoolVerify] baseline=%.1f current=%.1f drop=%.1f required=%.1f\n"),
                                baselineTemp, currentTemp, drop, requiredDrop);
                if (drop >= requiredDrop) {
                    verified = true;
                    active   = false;
                    _state   = CVState::VERIFIED;
                    Serial.println(F("[CoolVerify] Verified — cooling confirmed"));
                } else {
                    retryCount++;
                    Serial.printf_P(PSTR("[CoolVerify] Retry #%u — retransmitting AC ON\n"),
                                    (unsigned)retryCount);
                    ac.powerOn();
                    _state     = CVState::RETRYING;
                    _waitStart = millis();
                }
                break;
            }

            case CVState::RETRYING:
                // Brief pause after retransmit before going back to WAITING
                if ((now - _waitStart) >= 2000UL) {
                    _state     = CVState::WAITING;
                    _waitStart = millis();
                    Serial.println(F("[CoolVerify] Back to WAITING after retry"));
                }
                break;

            case CVState::VERIFIED:
            case CVState::IDLE:
            default:
                break;
        }
    }

    // ── start() — call when AC transitions OFF → ON ───────────────────────
    void start(float currentTemp) {
        if (!enabled) return;
        if (!(_state == CVState::IDLE || _state == CVState::VERIFIED)) return;

        baselineTemp = currentTemp;
        baselineTime = millis();
        _waitStart   = millis();
        active       = true;
        verified     = false;
        retryCount   = 0;
        _state       = CVState::WAITING;

        Serial.printf_P(PSTR("[CoolVerify] Started — baseline=%.1f C  interval=%lu min  drop=%.1f C\n"),
                        baselineTemp,
                        (unsigned long)(intervalMs / 60000UL),
                        requiredDrop);
    }

    // ── cancel() — call when AC is turned OFF ─────────────────────────────
    void cancel() {
        if (_state == CVState::IDLE) return;
        _state   = CVState::IDLE;
        active   = false;
        verified = false;
        Serial.println(F("[CoolVerify] Cancelled"));
    }

    // ── statusString() — human-readable state for /api/cooling ───────────
    const char* statusString() const {
        switch (_state) {
            case CVState::IDLE:     return "idle";
            case CVState::WAITING:  return "waiting";
            case CVState::CHECKING: return "checking";
            case CVState::VERIFIED: return "verified";
            case CVState::RETRYING: return "retrying";
            default:                return "unknown";
        }
    }

private:
    CVState  _state     = CVState::IDLE;
    uint32_t _waitStart = 0;
};


static inline void powerOnWithCV(ACController&        ac,
                                  SensorManager&       sensor,
                                  CoolingVerification& cv) {
    ac.powerOn();
    if (cv.enabled) {
        if (sensor.isValid()) {
            cv.start(sensor.getTemp());
        } else {
            Serial.println(F("[CoolVerify] Sensor invalid at power-on — CV not started"));
        }
    }
}

/*
 * powerOffWithCV()
 *   1. Calls ac.powerOff() — transmits IR OFF burst.
 *   2. Cancels any active cooling verification.
 */
static inline void powerOffWithCV(ACController&        ac,
                                   CoolingVerification& cv) {
    ac.powerOff();
    cv.cancel();
}
