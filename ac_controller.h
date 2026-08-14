#pragma once

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Coolix.h>
#include "config.h"

// ── AC State ──────────────────────────────────────────────────────────────────
struct ACState {
    bool    power = false;
    uint8_t temp  = AC_DEFAULT_TEMP;
    uint8_t mode  = kCoolixCool;
    uint8_t fan   = kCoolixFanAuto;
    bool    swing = false;
    bool    turbo = false;
    bool    sleep = false;
};

// ── Pending IR command ────────────────────────────────────────────────────────
// SET_PARAMS covers temp/mode/fan/swing/turbo/sleep changes.
// The new values are already written into _state before the command is queued,
// so _execSetParams() just calls _transmitFull() which reads _state.
enum class IRCmd : uint8_t {
    NONE,
    POWER_ON,
    POWER_OFF,
    SET_PARAMS   // re-transmit full state (temp, mode, fan, special modes)
};

// ── ACController ─────────────────────────────────────────────────────────────
class ACController {
public:
    ACController() : _ac(IR_LED_PIN), _pending(IRCmd::NONE) {}

    void begin() {
        _ac.begin();
        Serial.println(F("[AC] IRCoolixAC ready (D5 / GPIO14)"));
    }


    void tick() {
        if (_pending == IRCmd::NONE) return;
        IRCmd cmd = _pending;
        _pending = IRCmd::NONE;
        switch (cmd) {
            case IRCmd::POWER_ON:   _execPowerOn();   break;
            case IRCmd::POWER_OFF:  _execPowerOff();  break;
            case IRCmd::SET_PARAMS: _execSetParams();  break;
            default: break;
        }
    }

    bool hasPending() const { return _pending != IRCmd::NONE; }

    // ── Power ──────────────────────────────────────────────────────────────
    void powerOn() {
        _state.power = true;
        _state.temp  = AC_DEFAULT_TEMP;
        _state.mode  = kCoolixCool;
        _state.fan   = kCoolixFanAuto;
        _state.swing = false;
        _state.turbo = false;
        _state.sleep = false;
        _pending = IRCmd::POWER_ON;
        Serial.println(F("[AC] Power ON queued"));
    }

    void powerOff() {
        _state.power = false;
        _pending = IRCmd::POWER_OFF;
        Serial.println(F("[AC] Power OFF queued"));
    }

    // ── Temperature ────────────────────────────────────────────────────────
    bool setTemp(uint8_t t) {
        if (t < AC_TEMP_MIN || t > AC_TEMP_MAX) return false;
        _state.temp = t;
        _pending = IRCmd::SET_PARAMS;
        Serial.printf_P(PSTR("[AC] Temp -> %u C (queued)\n"), (unsigned)t);
        return true;
    }

    bool tempUp() {
        if (_state.temp >= AC_TEMP_MAX) return false;
        return setTemp(_state.temp + 1);
    }

    bool tempDown() {
        if (_state.temp <= AC_TEMP_MIN) return false;
        return setTemp(_state.temp - 1);
    }

    // ── Mode ───────────────────────────────────────────────────────────────
    bool setMode(uint8_t mode) {
        if (mode != kCoolixCool && mode != kCoolixDry &&
            mode != kCoolixAuto && mode != kCoolixHeat &&
            mode != kCoolixFan) {
            Serial.printf_P(PSTR("[AC] Invalid mode: %u\n"), (unsigned)mode);
            return false;
        }
        _state.mode = mode;
        _pending = IRCmd::SET_PARAMS;
        Serial.printf_P(PSTR("[AC] Mode -> %u (queued)\n"), (unsigned)mode);
        return true;
    }

    // ── Fan ────────────────────────────────────────────────────────────────
    bool setFan(uint8_t fan) {
        if (fan != kCoolixFanAuto && fan != kCoolixFanMin &&
            fan != kCoolixFanMed  && fan != kCoolixFanMax) {
            Serial.printf_P(PSTR("[AC] Invalid fan: %u\n"), (unsigned)fan);
            return false;
        }
        _state.fan = fan;
        _pending = IRCmd::SET_PARAMS;
        Serial.printf_P(PSTR("[AC] Fan -> %u (queued)\n"), (unsigned)fan);
        return true;
    }

    // ── Swing ──────────────────────────────────────────────────────────────
    void toggleSwing() {
        _state.swing = !_state.swing;
        if (_state.swing) { _state.turbo = false; _state.sleep = false; }
        _pending = IRCmd::SET_PARAMS;
        Serial.printf_P(PSTR("[AC] Swing -> %s (queued)\n"),
                        _state.swing ? "ON" : "OFF");
    }

    // ── Turbo ──────────────────────────────────────────────────────────────
    void toggleTurbo() {
        _state.turbo = !_state.turbo;
        if (_state.turbo) { _state.swing = false; _state.sleep = false; }
        _pending = IRCmd::SET_PARAMS;
        Serial.printf_P(PSTR("[AC] Turbo -> %s (queued)\n"),
                        _state.turbo ? "ON" : "OFF");
    }

    // ── Sleep ──────────────────────────────────────────────────────────────
    void toggleSleep() {
        _state.sleep = !_state.sleep;
        if (_state.sleep) { _state.swing = false; _state.turbo = false; }
        _pending = IRCmd::SET_PARAMS;
        Serial.printf_P(PSTR("[AC] Sleep -> %s (queued)\n"),
                        _state.sleep ? "ON" : "OFF");
    }

    // ── Accessors ──────────────────────────────────────────────────────────
    const ACState& getState() const { return _state; }

private:
    IRCoolixAC _ac;
    ACState    _state;
    IRCmd      _pending;

    // ── IR execution — 

    void _execPowerOn() {
        _applyState();
        for (int i = 0; i < 5; i++) {
            _ac.send();
            delay(200);
        }
        Serial.println(F("[AC] Power ON transmitted"));
    }

    void _execPowerOff() {
        _ac.off();
        _ac.send();
        Serial.println(F("[AC] Power OFF transmitted"));
    }

    void _execSetParams() {
        if (!_state.power) return;
        _applyState();
        if (_state.swing) _ac.setSwing();
        if (_state.turbo) _ac.setTurbo();
        if (_state.sleep) _ac.setSleep();
        _ac.send();
        Serial.println(F("[AC] Params transmitted"));
    }

    // Write _state fields into the IRCoolixAC object (does not send).
    void _applyState() {
        if (_state.power) { _ac.on(); } else { _ac.off(); }
        _ac.setTemp(_state.temp);
        _ac.setMode(_state.mode);
        _ac.setFan(_state.fan);
    }
};
