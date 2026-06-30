#pragma once

#include <ESP8266WebServer.h>
#include <LittleFS.h>

class StaticFileHandler {
public:
    explicit StaticFileHandler(ESP8266WebServer& server);

    bool serve(const char* path, const char* contentType);
    void sendNotFound();

private:
    ESP8266WebServer& server_;
};
