#pragma once
/*
 * routes.h  –  ESPAsyncWebServer route handlers (SmartAC2 — independent unit)
 * ─────────────────────────────────────────────────────────────────────────────
 * Hardware  : NodeMCU ESP8266 (ESP-12E/ESP-12F)
 *
 * Authentication model (persistent cookie):
 *   POST /api/login
 *     → validates credentials
 *     → generates session token via AuthManager::login()
 *     → sends:  Set-Cookie: smartac2_session=<token>; Path=/; Max-Age=86400;
 *                           HttpOnly; SameSite=Lax
 *     → browser stores cookie and sends it automatically on every request
 *
 *   Every protected route calls CHECK_AUTH(req) which:
 *     → reads the Cookie header
 *     → extracts the token via AuthManager::extractCookie()
 *     → validates via AuthManager::isValid()
 *     → returns 401 JSON if invalid (JS redirects to / on 401)
 *
 *   GET /  (login page):
 *     → if valid cookie already present → redirect to /home immediately
 *     → otherwise → serve LOGIN_HTML
 *
 *   POST /api/logout
 *     → removes session from AuthManager
 *     → sends:  Set-Cookie: smartac2_session=; Path=/; Max-Age=0;
 *                           HttpOnly; SameSite=Lax
 *     → browser deletes the cookie
 *
 *   All HTML page routes use CHECK_AUTH; on 401 the browser's JS
 *   redirects to / so the user sees the login page.
 *   Every fetch() call in every HTML page includes credentials:'include'
 *   so the browser sends the cookie automatically.
 *
 * CHANGED (ESP32 → ESP8266):
 *   - PsychicHttpServer / PsychicRequest* / esp_err_t  → ESPAsyncWebServer
 *   - <WiFi.h>       → <ESP8266WiFi.h>
 *   - <Preferences.h>→ ConfigStore (LittleFS)
 *   - Update.begin() → ESP8266 Update library
 *   - Removed all HTTPS/TLS code
 *   - Cookie: removed "; Secure" attribute (HTTP only on ESP8266)
 *
 * BUG FIX – body capture (root cause of login failure):
 *   The global server.onRequestBody() callback is NOT guaranteed to fire
 *   before the route handler in ESPAsyncWebServer (old fork, v3.x).
 *   When a small JSON body arrives in the same TCP segment as the headers,
 *   the library may dispatch the route handler immediately, leaving
 *   _tempObject NULL so getBody() returns "". deserializeJson("") succeeds
 *   with an empty document, doc["username"]|"" yields "", and auth.login()
 *   returns an empty token → 401 even with correct credentials.
 *
 *   Fix: replace the global onRequestBody + getBody() pattern with the
 *   4-argument server.on() overload that delivers body data directly to
 *   each route's own body handler, which ESPAsyncWebServer guarantees fires
 *   before the completion (request) handler for that route.
 *
 *   Pattern used for every JSON POST/PUT:
 *     server.on(uri, method,
 *       [captures](AsyncWebServerRequest* req) {
 *           // _tempObject is now always populated before this fires
 *           String body = getBody(req);
 *           ...
 *       },
 *       nullptr,   // no file-upload handler
 *       bodyCollector   // route-local body accumulator
 *     );
 *
 *   Routes that send no body (GET, and no-body POSTs like power/toggle)
 *   use the 3-argument server.on() as before — no change needed.
 *
 * BUG FIX – per-schedule routes (regex unsupported on ESP8266 toolchain):
 *   The original PUT /:id, DELETE /:id, and POST /:id/enable routes used
 *   ESPAsyncWebServer's regex server.on() overload ("^\\/api\\/...$").
 *   That worked on ESP32 because the ESP32 Arduino core's libstdc++ ships a
 *   working std::regex. The ESP8266 Arduino core's xtensa-lx106-elf-g++
 *   toolchain does NOT ship a working std::regex implementation, so these
 *   routes silently fail to match — every request falls through to
 *   onNotFound() and the frontend sees 404 "not found" for Edit, Delete,
 *   and Enable/Disable on individual schedules.
 *
 *   Fix: replace the three regex routes with a single custom AsyncWebHandler
 *   (ScheduleIdHandler, below) that matches "/api/auto/schedules/<digits>"
 *   and "/api/auto/schedules/<digits>/enable" via plain string parsing —
 *   no <regex> dependency at all.
 */

#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "ac_controller.h"
#include "sensor.h"
#include "auth.h"
#include "scheduler.h"
#include "cooling_verification.h"
#include "html_pages.h"
#include "html_pages2.h"
#include "config.h"
#include "config_store.h"
#include <Ticker.h>
static Ticker _restartTimer;

// Forward declaration – defined in ThermalEdge_AC2.ino
void saveWiFiCreds(const String& ssid, const String& pass);

// ─────────────────────────────────────────────────────────────────────────────
// Cookie helpers
// ─────────────────────────────────────────────────────────────────────────────

// Extract session token from the incoming Cookie header.
static String getSessionToken(AsyncWebServerRequest* req) {
    if (!req->hasHeader("Cookie")) return "";
    return AuthManager::extractCookie(req->header("Cookie"));
}

// Build the Set-Cookie string for login (Max-Age = SESSION_MAXAGE).
// No "; Secure" — project uses HTTP.
static String buildSetCookie(const String& token) {
    String c;
    c.reserve(128);
    c  = SESSION_COOKIE;
    c += '=';
    c += token;
    c += F("; Path=/; Max-Age=");
    c += String(SESSION_MAXAGE);
    c += F("; HttpOnly; SameSite=Lax");
    return c;
}

// Build the Set-Cookie string for logout (Max-Age=0 clears the cookie).
static String buildClearCookie() {
    String c;
    c.reserve(80);
    c  = SESSION_COOKIE;
    c += F("=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax");
    return c;
}

// Send a JSON response with a Set-Cookie header.
static void sendWithCookie(AsyncWebServerRequest* req,
                           const String& cookie,
                           const char* body) {
    AsyncResponseStream* resp =
        req->beginResponseStream(F("application/json"));
    resp->addHeader(F("Set-Cookie"), cookie);
    resp->print(body);
    req->send(resp);
}

// Retrieve the request body accumulated by the route-local body handler.
// Returns empty string if no body was captured (_tempObject is NULL).
static String getBody(AsyncWebServerRequest* req) {
    if (!req->_tempObject) return String();
    return String((char*)req->_tempObject);
}

// ─────────────────────────────────────────────────────────────────────────────
// Route-local body accumulator
// ─────────────────────────────────────────────────────────────────────────────
// Pass this as the 5th argument (ArBodyHandlerFunction) to server.on() for
// every route that needs to read the request body.  ESPAsyncWebServer
// guarantees this callback fires (and completes) before the request handler.
//
// Uses req->_tempObject to pass the heap-allocated String to the handler.
// The request destructor does NOT free _tempObject, so the handler must
// call freeBody(req) after it has consumed the data, or let it leak if the
// ESP8266's heap budget allows (fine for low-traffic embedded use).


// Free the body buffer after the handler has consumed it.
static void freeBody(AsyncWebServerRequest* req) {
    if (req->_tempObject) {
        free(req->_tempObject);
        req->_tempObject = nullptr;
    }
}

static void bodyCollector(AsyncWebServerRequest* req,
                          uint8_t* data, size_t len,
                          size_t index, size_t total) {
    if (index == 0) {
        if (req->_tempObject) { free(req->_tempObject); }
        char* buf = (char*)malloc(total + 1);
        if (!buf) return;
        buf[0] = '\0';
        req->_tempObject = buf;
    }
    if (req->_tempObject) {
        memcpy((char*)req->_tempObject + index, data, len);
        ((char*)req->_tempObject)[index + len] = '\0';
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Auth guard macro
// ─────────────────────────────────────────────────────────────────────────────
// Used in every protected route.
// Returns HTTP 401 JSON so the frontend JS can detect it and redirect to /.
#define CHECK_AUTH(req) \
    do { \
        if (!auth.isValid(getSessionToken(req))) { \
            req->send(401, F("application/json"), \
                      F("{\"error\":\"unauthorized\"}")); \
            return; \
        } \
    } while(0)

// ─────────────────────────────────────────────────────────────────────────────
// Status JSON builder
// ─────────────────────────────────────────────────────────────────────────────
static String buildStatusJson(ACController& ac, SensorManager& sensor) {
    StaticJsonDocument<512> doc;
    const ACState& s = ac.getState();
    doc["power"]   = s.power;
    doc["temp"]    = s.temp;
    doc["mode"]    = (int)s.mode;
    doc["fan"]     = (int)s.fan;
    doc["swing"]   = s.swing;
    doc["turbo"]   = s.turbo;
    doc["sleep"]   = s.sleep;
    doc["tempMin"] = AC_TEMP_MIN;
    doc["tempMax"] = AC_TEMP_MAX;
    if (sensor.isValid()) {
        doc["roomTemp"] = (float)(round(sensor.getTemp() * 10.0f) / 10.0f);
        doc["sensorOk"] = true;
    } else {
        doc["roomTemp"] = nullptr;
        doc["sensorOk"] = false;
    }
    doc["wifiOk"] = (WiFi.status() == WL_CONNECTED);
    doc["ip"]     = WiFi.localIP().toString();
    String out;
    serializeJson(doc, out);
    return out;
}

// Inject a "message" field and send as 200 JSON.
static void respondWithState(AsyncWebServerRequest* req,
                              ACController& ac,
                              SensorManager& sensor,
                              const char* message) {
    String body = buildStatusJson(ac, sensor);
    body.remove(body.length() - 1);   // strip closing '}'
    body += F(",\"message\":\"");
    body += message;
    body += F("\"}");
    req->send(200, F("application/json"), body);
}

// ─────────────────────────────────────────────────────────────────────────────
// ScheduleIdHandler  –  handles PUT/DELETE on /api/auto/schedules/<id>
//                       and POST on /api/auto/schedules/<id>/enable
// ─────────────────────────────────────────────────────────────────────────────
// Replaces the regex-based server.on() routes that don't work on ESP8266
// (xtensa-lx106-elf-g++ has no working std::regex). Matches paths by plain
// string parsing instead. Registered via server.addHandler() below.
class ScheduleIdHandler : public AsyncWebHandler {
public:
    ScheduleIdHandler(AuthManager& auth, SchedulerManager& scheduler)
        : _auth(auth), _scheduler(scheduler) {}

    bool canHandle(AsyncWebServerRequest* req) override {
        static const char PREFIX[] = "/api/auto/schedules/";
        String url = req->url();
        if (!url.startsWith(PREFIX)) return false;

        String rest = url.substring(strlen(PREFIX));
        bool isEnable = rest.endsWith("/enable");
        if (isEnable) rest = rest.substring(0, rest.length() - 7); // strip "/enable"

        if (rest.length() == 0) return false;
        for (size_t i = 0; i < rest.length(); i++) {
            if (!isDigit(rest[i])) return false;
        }
        req->addInterestingHeader("ANY");
        if (isEnable) return req->method() == HTTP_POST;
        return req->method() == HTTP_PUT || req->method() == HTTP_DELETE;
    }

    // We need access to the request body (PUT / enable), so this handler
    // is not "trivial" — ESPAsyncWebServer will call handleBody() for us.
    bool isRequestHandlerTrivial() override { return false; }

    void handleBody(AsyncWebServerRequest* req, uint8_t* data, size_t len,
                     size_t index, size_t total) override {
        bodyCollector(req, data, len, index, total);
    }

    void handleRequest(AsyncWebServerRequest* req) override {
        static const char PREFIX[] = "/api/auto/schedules/";
        String url  = req->url();
        String rest = url.substring(strlen(PREFIX));
        bool isEnable = rest.endsWith("/enable");
        if (isEnable) rest = rest.substring(0, rest.length() - 7);
        uint8_t i = (uint8_t)rest.toInt();

        AuthManager&      auth      = _auth;       // for CHECK_AUTH macro
        SchedulerManager& scheduler = _scheduler;

        if (req->method() == HTTP_DELETE) {
            freeBody(req); // discard any (unexpected) body that was collected
            CHECK_AUTH(req);
            if (!scheduler.deleteSchedule(i)) {
                req->send(404, F("application/json"),
                          F("{\"error\":\"schedule not found\"}"));
                return;
            }
            req->send(200, F("application/json"), F("{\"ok\":true}"));
            return;
        }

        // PUT (update) and POST .../enable both carry a JSON body.
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);

        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }

        if (isEnable) {
            bool en = doc["enabled"] | true;
            if (!scheduler.setEnabled(i, en)) {
                req->send(404, F("application/json"),
                          F("{\"error\":\"schedule not found\"}"));
                return;
            }
            req->send(200, F("application/json"), F("{\"ok\":true}"));
            return;
        }

        // PUT — update schedule
        Schedule s;
        s.startHH = doc["startHH"] | 0; s.startMM = doc["startMM"] | 0;
        s.stopHH  = doc["stopHH"]  | 0; s.stopMM  = doc["stopMM"]  | 0;
        s.enabled = doc["enabled"] | true;
        if (s.startHH > 23 || s.startMM > 59 ||
            s.stopHH  > 23 || s.stopMM  > 59) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"invalid time values\"}"));
            return;
        }
        if (!scheduler.updateSchedule(i, s)) {
            req->send(404, F("application/json"),
                      F("{\"error\":\"schedule not found\"}"));
            return;
        }
        req->send(200, F("application/json"), F("{\"ok\":true}"));
    }

private:
    AuthManager&      _auth;
    SchedulerManager& _scheduler;
};

// ─────────────────────────────────────────────────────────────────────────────
// registerRoutes
// ─────────────────────────────────────────────────────────────────────────────
void registerRoutes(AsyncWebServer&      server,
                    ACController&        ac,
                    SensorManager&       sensor,
                    AuthManager&         auth,
                    SchedulerManager&    scheduler,
                    CoolingVerification& cv) {

    // ── GET / ──────────────────────────────────────────────────────────────
    // If a valid session cookie is already present, redirect to /home so the
    // user is never shown the login page again until the cookie expires.
    server.on("/", HTTP_GET,
    [&auth](AsyncWebServerRequest* req) {
        if (auth.isValid(getSessionToken(req))) {
            req->redirect(F("/home"));
            return;
        }
        req->send_P(200, F("text/html"), LOGIN_HTML);
    });

    // ── GET /home ──────────────────────────────────────────────────────────
    server.on("/home", HTTP_GET,
    [&auth](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        req->send_P(200, F("text/html"), HOME_HTML);
    });

    // ── GET /dashboard ─────────────────────────────────────────────────────
    server.on("/dashboard", HTTP_GET,
    [&auth](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        req->send_P(200, F("text/html"), DASHBOARD_HTML);
    });

    // ── GET /auto ──────────────────────────────────────────────────────────
    server.on("/auto", HTTP_GET,
    [&auth](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        req->send_P(200, F("text/html"), AUTO_HTML);
    });

    // ── GET /temp ──────────────────────────────────────────────────────────
    server.on("/temp", HTTP_GET,
    [&auth](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        req->send_P(200, F("text/html"), TEMP_HTML);
    });

    // ── GET /settings ──────────────────────────────────────────────────────
    server.on("/settings", HTTP_GET,
    [&auth](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        req->send_P(200, F("text/html"), SETTINGS_HTML);
    });

    // ── POST /api/login ────────────────────────────────────────────────────
    // Body: { "username": "...", "password": "..." }
    // Uses 4-arg server.on() so bodyCollector is guaranteed to run first.
    server.on("/api/login", HTTP_POST,
    [&auth](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"bad request\"}"));
            return;
        }
        String token = auth.login(doc["username"] | "", doc["password"] | "");
        if (token.isEmpty()) {
            req->send(401, F("application/json"),
                      F("{\"error\":\"invalid credentials\"}"));
            return;
        }
        sendWithCookie(req, buildSetCookie(token), "{\"ok\":true}");
    },
    nullptr,        // no file-upload handler
    bodyCollector   // ← body arrives here before the handler above fires
    );

    // ── POST /api/logout ───────────────────────────────────────────────────
    server.on("/api/logout", HTTP_POST,
    [&auth](AsyncWebServerRequest* req) {
        auth.logout(getSessionToken(req));
        sendWithCookie(req, buildClearCookie(), "{\"ok\":true}");
    });

    // ── GET /api/status ────────────────────────────────────────────────────
    server.on("/api/status", HTTP_GET,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        req->send(200, F("application/json"), buildStatusJson(ac, sensor));
    });

    // ── GET /api/history ───────────────────────────────────────────────────
    server.on("/api/history", HTTP_GET,
    [&auth, &sensor](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        req->send(200, F("application/json"), sensor.getHistoryJson());
    });

    // ── POST /api/poweron ──────────────────────────────────────────────────
    server.on("/api/poweron", HTTP_POST,
    [&auth, &ac, &sensor, &cv](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        powerOnWithCV(ac, sensor, cv);
        respondWithState(req, ac, sensor, "AC turned ON");
    });

    // ── POST /api/poweroff ─────────────────────────────────────────────────
    server.on("/api/poweroff", HTTP_POST,
    [&auth, &ac, &sensor, &cv](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        powerOffWithCV(ac, cv);
        respondWithState(req, ac, sensor, "AC turned OFF");
    });

    // ── POST /api/tempup ───────────────────────────────────────────────────
    server.on("/api/tempup", HTTP_POST,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        bool ok = ac.tempUp();
        respondWithState(req, ac, sensor,
            ok ? "Temperature increased" : "Already at maximum");
    });

    // ── POST /api/tempdown ─────────────────────────────────────────────────
    server.on("/api/tempdown", HTTP_POST,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        bool ok = ac.tempDown();
        respondWithState(req, ac, sensor,
            ok ? "Temperature decreased" : "Already at minimum");
    });

    // ── POST /api/settemp ──────────────────────────────────────────────────
    server.on("/api/settemp", HTTP_POST,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        int t = doc["temp"] | -1;
        if (t < 0) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"missing temp field\"}"));
            return;
        }
        bool ok = ac.setTemp((uint8_t)t);
        char msg[44];
        snprintf(msg, sizeof(msg),
                 ok ? "Temperature set to %d C" : "Invalid temperature: %d", t);
        respondWithState(req, ac, sensor, msg);
    },
    nullptr, bodyCollector);

    // ── POST /api/setmode ──────────────────────────────────────────────────
    server.on("/api/setmode", HTTP_POST,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        int m = doc["mode"] | -1;
        if (m < 0) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"missing mode field\"}"));
            return;
        }
        bool ok = ac.setMode((uint8_t)m);
        respondWithState(req, ac, sensor,
            ok ? "Mode updated" : "Invalid mode");
    },
    nullptr, bodyCollector);

    // ── POST /api/setfan ───────────────────────────────────────────────────
    server.on("/api/setfan", HTTP_POST,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        int f = doc["fan"] | -1;
        if (f < 0) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"missing fan field\"}"));
            return;
        }
        bool ok = ac.setFan((uint8_t)f);
        respondWithState(req, ac, sensor,
            ok ? "Fan speed updated" : "Invalid fan speed");
    },
    nullptr, bodyCollector);

    // ── POST /api/swing ────────────────────────────────────────────────────
    server.on("/api/swing", HTTP_POST,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        ac.toggleSwing();
        respondWithState(req, ac, sensor,
            ac.getState().swing ? "Swing ON" : "Swing OFF");
    });

    // ── POST /api/turbo ────────────────────────────────────────────────────
    server.on("/api/turbo", HTTP_POST,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        ac.toggleTurbo();
        respondWithState(req, ac, sensor,
            ac.getState().turbo ? "Turbo ON" : "Turbo OFF");
    });

    // ── POST /api/sleep ────────────────────────────────────────────────────
    server.on("/api/sleep", HTTP_POST,
    [&auth, &ac, &sensor](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        ac.toggleSleep();
        respondWithState(req, ac, sensor,
            ac.getState().sleep ? "Sleep ON" : "Sleep OFF");
    });

    // ── GET /api/ntp ───────────────────────────────────────────────────────
    server.on("/api/ntp", HTTP_GET,
    [&auth, &scheduler](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        StaticJsonDocument<128> doc;
        doc["synced"] = scheduler.isNTPSynced();
        doc["time"]   = scheduler.getLocalTimeStr();
        String out; serializeJson(doc, out);
        req->send(200, F("application/json"), out);
    });

    // ── GET /api/auto/settings ─────────────────────────────────────────────
    server.on("/api/auto/settings", HTTP_GET,
    [&auth, &scheduler](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        const GlobalACSettings& g = scheduler.getGlobalSettings();
        StaticJsonDocument<128> doc;
        doc["temp"]  = g.temp;      doc["mode"]  = (int)g.mode;
        doc["fan"]   = (int)g.fan;  doc["swing"] = g.swing;
        doc["turbo"] = g.turbo;     doc["sleep"] = g.sleep;
        String out; serializeJson(doc, out);
        req->send(200, F("application/json"), out);
    });

    // ── POST /api/auto/settings ────────────────────────────────────────────
    server.on("/api/auto/settings", HTTP_POST,
    [&auth, &scheduler](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        GlobalACSettings g;
        g.temp  = doc["temp"]  | (int)AC_DEFAULT_TEMP;
        g.mode  = doc["mode"]  | (int)kCoolixCool;
        g.fan   = doc["fan"]   | (int)kCoolixFanAuto;
        g.swing = doc["swing"] | false;
        g.turbo = doc["turbo"] | false;
        g.sleep = doc["sleep"] | false;
        if (!scheduler.setGlobalSettings(g)) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"invalid settings values\"}"));
            return;
        }
        StaticJsonDocument<128> out;
        out["temp"]  = g.temp;     out["mode"]  = (int)g.mode;
        out["fan"]   = (int)g.fan; out["swing"] = g.swing;
        out["turbo"] = g.turbo;    out["sleep"] = g.sleep;
        String body; serializeJson(out, body);
        req->send(200, F("application/json"), body);
    },
    nullptr, bodyCollector);

    // ── GET /api/auto/schedules ────────────────────────────────────────────
    server.on("/api/auto/schedules", HTTP_GET,
    [&auth, &scheduler](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        DynamicJsonDocument doc(2048);
        JsonArray arr = doc.createNestedArray("schedules");
        uint8_t cnt = scheduler.getScheduleCount();
        for (uint8_t i = 0; i < cnt; i++) {
            const Schedule* s = scheduler.getSchedule(i);
            if (!s) continue;
            JsonObject o = arr.createNestedObject();
            o["startHH"] = s->startHH; o["startMM"] = s->startMM;
            o["stopHH"]  = s->stopHH;  o["stopMM"]  = s->stopMM;
            o["enabled"] = s->enabled;
        }
        String body; serializeJson(doc, body);
        req->send(200, F("application/json"), body);
    });

    // ── POST /api/auto/schedules ───────────────────────────────────────────
    server.on("/api/auto/schedules", HTTP_POST,
    [&auth, &scheduler](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        Schedule s;
        s.startHH = doc["startHH"] | 0; s.startMM = doc["startMM"] | 0;
        s.stopHH  = doc["stopHH"]  | 0; s.stopMM  = doc["stopMM"]  | 0;
        s.enabled = doc["enabled"] | true;
        if (s.startHH > 23 || s.startMM > 59 ||
            s.stopHH  > 23 || s.stopMM  > 59) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"invalid time values\"}"));
            return;
        }
        uint8_t idx = scheduler.addSchedule(s);
        if (idx == 0xFF) {
            req->send(507, F("application/json"),
                      F("{\"error\":\"max schedules reached\"}"));
            return;
        }
        StaticJsonDocument<64> out;
        out["ok"] = true; out["idx"] = idx;
        String body; serializeJson(out, body);
        req->send(201, F("application/json"), body);
    },
    nullptr, bodyCollector);

    // ── Per-schedule PUT / DELETE / enable ─────────────────────────────────
    // Handled by ScheduleIdHandler (above) — NOT regex server.on() routes,
    // because std::regex is unavailable/broken on the ESP8266 toolchain.
    // This single handler matches:
    //   PUT    /api/auto/schedules/<id>
    //   DELETE /api/auto/schedules/<id>
    //   POST   /api/auto/schedules/<id>/enable
    server.addHandler(new ScheduleIdHandler(auth, scheduler));

    // ── GET /api/tempmode ──────────────────────────────────────────────────
    server.on("/api/tempmode", HTTP_GET,
    [&auth, &scheduler](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        const TempModeSettings& t = scheduler.getTempMode();
        StaticJsonDocument<128> doc;
        doc["enabled"]       = t.enabled;
        doc["threshold"]     = t.threshold;
        doc["coolingTemp"]   = t.coolingTemp;
        doc["coolingActive"] = scheduler.isCoolingActive();
        String out; serializeJson(doc, out);
        req->send(200, F("application/json"), out);
    });

    // ── POST /api/tempmode ─────────────────────────────────────────────────
    server.on("/api/tempmode", HTTP_POST,
    [&auth, &scheduler](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        TempModeSettings t;
        t.enabled     = doc["enabled"]    | false;
        t.threshold   = doc["threshold"]  | 30;
        t.coolingTemp = doc["coolingTemp"] | 25;
        if (!scheduler.setTempMode(t)) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"invalid temperature values\"}"));
            return;
        }
        req->send(200, F("application/json"), F("{\"ok\":true}"));
    },
    nullptr, bodyCollector);

    // ── GET /api/settings ──────────────────────────────────────────────────
    server.on("/api/settings", HTTP_GET,
    [&auth](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        String ssid, pass;
        if (!ConfigStore::loadWiFiCreds(ssid, pass)) ssid = WIFI_SSID;
        StaticJsonDocument<128> doc;
        doc["username"] = auth.getUsername();
        doc["wifiSSID"] = ssid;
        String out; serializeJson(doc, out);
        req->send(200, F("application/json"), out);
    });

    // ── POST /api/settings/credentials ────────────────────────────────────
    server.on("/api/settings/credentials", HTTP_POST,
    [&auth](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<512> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        String newUser = doc["username"] | "";
        String newPass = doc["password"] | "";
        if (!auth.setCredentials(newUser, newPass)) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"empty username or password\"}"));
            return;
        }
        // setCredentials() calls logoutAll() — clear the browser cookie too
        sendWithCookie(req, buildClearCookie(), "{\"ok\":true}");
    },
    nullptr, bodyCollector);

    // ── POST /api/settings/wifi ────────────────────────────────────────────
    server.on("/api/settings/wifi", HTTP_POST,
    [&auth](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<512> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        String ssid = doc["ssid"]     | "";
        String pass = doc["password"] | "";
        if (ssid.isEmpty()) {
            req->send(400, F("application/json"), F("{\"error\":\"ssid required\"}"));
            return;
        }
        saveWiFiCreds(ssid, pass);
        req->send(200, F("application/json"),
                  F("{\"ok\":true,\"message\":\"WiFi saved. Restarting.\"}"));
        _restartTimer.once_ms(1500, []() { ESP.restart(); });
    },
    nullptr, bodyCollector);

    // ── POST /api/settings/factory-reset ──────────────────────────────────
    server.on("/api/settings/factory-reset", HTTP_POST,
    [&auth, &scheduler](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        scheduler.factoryReset();
        ConfigStore::factoryReset();
        Serial.println(F("[Settings] Factory reset complete"));
        req->send(200, F("application/json"),
                  F("{\"ok\":true,\"message\":\"Factory reset. Restarting.\"}"));
        _restartTimer.once_ms(1500, []() { ESP.restart(); });
    });

    // ── GET /api/cooling ───────────────────────────────────────────────────
    server.on("/api/cooling", HTTP_GET,
    [&auth, &cv, &sensor](AsyncWebServerRequest* req) {
        CHECK_AUTH(req);
        StaticJsonDocument<256> doc;
        doc["enabled"]      = cv.enabled;
        doc["active"]       = cv.active;
        doc["verified"]     = cv.verified;
        doc["retryCount"]   = cv.retryCount;
        doc["baselineTemp"] = cv.active
                              ? (float)(round(cv.baselineTemp * 10.0f) / 10.0f)
                              : 0.0f;
        doc["currentTemp"]  = sensor.isValid()
                              ? (float)(round(sensor.getTemp() * 10.0f) / 10.0f)
                              : 0.0f;
        doc["intervalMin"]  = (uint32_t)(cv.intervalMs / 60000UL);
        doc["requiredDrop"] = cv.requiredDrop;
        doc["status"]       = cv.statusString();
        doc["elapsedSec"]   = cv.active
                              ? (uint32_t)((millis() - cv.baselineTime) / 1000UL)
                              : 0;
        String out; serializeJson(doc, out);
        req->send(200, F("application/json"), out);
    });

    // ── POST /api/cooling ──────────────────────────────────────────────────
    server.on("/api/cooling", HTTP_POST,
    [&auth, &cv](AsyncWebServerRequest* req) {
        String bodyStr = getBody(req);
        freeBody(req);
        CHECK_AUTH(req);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, bodyStr) != DeserializationError::Ok) {
            req->send(400, F("application/json"), F("{\"error\":\"bad request\"}"));
            return;
        }
        cv.enabled    = doc["enabled"]      | cv.enabled;
        uint32_t iMin = doc["intervalMin"]  | (uint32_t)(cv.intervalMs / 60000UL);
        float    drop = doc["requiredDrop"] | cv.requiredDrop;
        if (iMin < 1 || iMin > 120) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"intervalMin must be 1-120\"}"));
            return;
        }
        if (drop < 0.1f || drop > 10.0f) {
            req->send(400, F("application/json"),
                      F("{\"error\":\"requiredDrop must be 0.1-10.0\"}"));
            return;
        }
        cv.intervalMs   = (unsigned long)iMin * 60000UL;
        cv.requiredDrop = drop;
        if (!cv.enabled) cv.cancel();
        Serial.printf_P(PSTR("[CoolVerify] Settings: enabled=%d interval=%u min drop=%.1f C\n"),
                        cv.enabled, iMin, cv.requiredDrop);
        req->send(200, F("application/json"), F("{\"ok\":true}"));
    },
    nullptr, bodyCollector);

    // ── POST /api/ota ──────────────────────────────────────────────────────
    // File upload — uses the 4-arg form with an upload handler (not bodyCollector).
    server.on("/api/ota", HTTP_POST,
    [](AsyncWebServerRequest* req) {
        bool ok = !Update.hasError();
        AsyncResponseStream* resp =
            req->beginResponseStream(F("application/json"));
        resp->setCode(ok ? 200 : 500);
        resp->addHeader(F("Connection"), F("close"));
        resp->print(ok ? "{\"ok\":true,\"message\":\"OTA complete. Rebooting.\"}"
                       : "{\"error\":\"OTA failed\"}");
        req->send(resp);
        if (ok) { delay(300); ESP.restart(); }
    },
    [&auth](AsyncWebServerRequest* req,
             const String& filename,
             size_t index,
             uint8_t* data,
             size_t len,
             bool final) {
        if (index == 0) {
            if (!auth.isValid(getSessionToken(req))) {
                Serial.println(F("[OTA] Rejected: not authenticated"));
                return;
            }
            Serial.printf_P(PSTR("[OTA] Start: %s\n"), filename.c_str());
            Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
        }
        if (Update.isRunning()) Update.write(data, len);
        if (final) {
            if (Update.end(true)) Serial.println(F("[OTA] Complete"));
            else                  Serial.println(F("[OTA] Failed"));
        }
    });

    // ── 404 ────────────────────────────────────────────────────────────────
    server.onNotFound([](AsyncWebServerRequest* req) {
        req->send(404, F("application/json"), F("{\"error\":\"not found\"}"));
    });
}
