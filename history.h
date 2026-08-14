#pragma once

#include <Arduino.h>

static constexpr uint8_t HISTORY_SIZE = 60;

struct SensorReading {
    uint32_t ts;    // millis() at time of reading
    float    temp;
};

class SensorHistory {
public:
    SensorHistory() : _head(0), _count(0) {}

    void push(float temp) {
        _buf[_head] = { millis(), temp };
        _head = (_head + 1) % HISTORY_SIZE;
        if (_count < HISTORY_SIZE) _count++;
    }

    String toJson() const {
        String out;
        out.reserve(1400);
        out = '[';
        const uint32_t now = millis();
        char entry[32];
        for (uint8_t i = 0; i < _count; i++) {
            const uint8_t idx = (uint8_t)((_head - _count + i + HISTORY_SIZE)
                                          % HISTORY_SIZE);
            const SensorReading& r = _buf[idx];
            const uint32_t secsAgo = (now - r.ts) / 1000UL;
            snprintf(entry, sizeof(entry),
                     "%s{\"ts\":%lu,\"t\":%.1f}",
                     (i > 0) ? "," : "",
                     (unsigned long)secsAgo,
                     r.temp);
            out += entry;
        }
        out += ']';
        return out;
    }

    uint8_t count() const { return _count; }

private:
    SensorReading _buf[HISTORY_SIZE];
    uint8_t       _head;
    uint8_t       _count;
};
