#pragma once

#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#include "history.h"

class SensorManager {
public:
    SensorManager()
        : _oneWire(ONE_WIRE_BUS)
        , _sensors(&_oneWire)
    {}

    void begin() {
        _sensors.begin();
        _sensors.setResolution(12);
        Serial.printf_P(PSTR("[Sensor] DS18B20 on GPIO%d  devices=%u\n"),
                        ONE_WIRE_BUS,
                        (unsigned)_sensors.getDeviceCount());
    }

    // Call from loop() every iteration.
    void update() {
        uint32_t now = millis();
        if ((now - _lastRead) < READ_INTERVAL_MS) return;
        _lastRead = now;

        _sensors.requestTemperatures();
        float t = _sensors.getTempCByIndex(0);

        if (t == DEVICE_DISCONNECTED_C || isnan(t) || t < -55.0f || t > 125.0f) {
            _valid = false;
            Serial.println(F("[Sensor] DS18B20 read failed – keeping previous value"));
            return;
        }

        _temp  = t;
        _valid = true;
        _history.push(_temp);
        Serial.printf_P(PSTR("[Sensor] DS18B20 Temp=%.2f C\n"), _temp);
    }

    float  getTemp()       const { return _valid ? _temp : -999.0f; }
    bool   isValid()       const { return _valid; }
    String getHistoryJson()const { return _history.toJson(); }

private:
    static constexpr uint32_t READ_INTERVAL_MS = 5000;

    OneWire           _oneWire;
    DallasTemperature _sensors;

    float         _temp     = 0.0f;
    bool          _valid    = false;
    uint32_t      _lastRead = 0;
    SensorHistory _history;
};
