#pragma once

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

namespace JsonResponse {

inline void sendOk(ESP8266WebServer& server, int code = 200) {
    server.send(code, "application/json", "{\"ok\":true}");
}

inline void sendError(ESP8266WebServer& server, int code, const char* error) {
    JsonDocument doc;
    doc["ok"] = false;
    doc["error"] = error;

    String response;
    serializeJson(doc, response);
    server.send(code, "application/json", response);
}

} // namespace JsonResponse
