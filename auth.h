#pragma once

#include <Arduino.h>
#include <map>
#include "config.h"
#include "config_store.h"

static inline uint32_t hw_random32() {
    return RANDOM_REG32;
}

// ── AuthManager ───────────────────────────────────────────────────────────────
class AuthManager {
public:
    void begin() {
        if (!ConfigStore::loadAuthCreds(_username, _password)) {
            _username = AUTH_USERNAME;
            _password = AUTH_PASSWORD;
            ConfigStore::saveAuthCreds(_username, _password);
        }
        Serial.printf_P(PSTR("[Auth] Credentials loaded (user=%s)\n"),
                        _username.c_str());
    }

    String login(const String& username, const String& password) {
        if (username != _username || password != _password) return "";
        String token = _generateToken();
        uint32_t expiry = millis() + (uint32_t)SESSION_MAXAGE * 1000UL;
        _sessions[token] = expiry;
        Serial.printf_P(PSTR("[Auth] Session created (...%s)\n"),
                        token.substring(28).c_str());
        return token;
    }

    // Returns true if token exists and has not expired.
    bool isValid(const String& token) {
        if (token.isEmpty()) return false;
        auto it = _sessions.find(token);
        if (it == _sessions.end()) return false;
        // Cast to signed so wrap-around subtraction works correctly
        if ((int32_t)(millis() - it->second) > 0) {
            _sessions.erase(it);
            return false;
        }
        return true;
    }

    void logout(const String& token) {
        if (!token.isEmpty()) _sessions.erase(token);
    }

    void logoutAll() { _sessions.clear(); }

    bool setCredentials(const String& newUser, const String& newPass) {
        if (newUser.isEmpty() || newPass.isEmpty()) return false;
        _username = newUser;
        _password = newPass;
        ConfigStore::saveAuthCreds(_username, _password);
        logoutAll();
        Serial.printf_P(PSTR("[Auth] Credentials updated (user=%s)\n"),
                        _username.c_str());
        return true;
    }

    const String& getUsername() const { return _username; }

    static String extractCookie(const String& cookieHeader) {
        String key = String(SESSION_COOKIE) + F("=");
        int idx = cookieHeader.indexOf(key);
        if (idx < 0) return "";
        idx += key.length();
        int end = cookieHeader.indexOf(';', idx);
        if (end < 0) end = (int)cookieHeader.length();
        String val = cookieHeader.substring(idx, end);
        val.trim();
        return val;
    }

private:
    String                     _username;
    String                     _password;
    std::map<String, uint32_t> _sessions;   // token → expiry millis

    String _generateToken() {
        char buf[33];
        snprintf(buf, sizeof(buf), "%08X%08X%08X%08X",
                 hw_random32(), hw_random32(),
                 hw_random32(), hw_random32());
        return String(buf);
    }
};
