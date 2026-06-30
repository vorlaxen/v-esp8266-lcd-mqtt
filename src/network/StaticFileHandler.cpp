#include "network/StaticFileHandler.h"

StaticFileHandler::StaticFileHandler(ESP8266WebServer& server) : server_(server) {}

bool StaticFileHandler::serve(const char* path, const char* contentType) {
    if (!LittleFS.exists(path)) {
        server_.send(404, "text/plain", "File not found");
        return false;
    }

    File file = LittleFS.open(path, "r");
    server_.streamFile(file, contentType);
    file.close();
    return true;
}

void StaticFileHandler::sendNotFound() {
    server_.send(404, "text/plain", "Not found");
}
