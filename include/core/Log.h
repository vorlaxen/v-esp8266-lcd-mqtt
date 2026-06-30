#pragma once

#include <Arduino.h>
#include <stdarg.h>

namespace Log {

inline void info(const char* message) {
    Serial.print("[INFO] ");
    Serial.println(message);
}

inline void warn(const char* message) {
    Serial.print("[WARN] ");
    Serial.println(message);
}

inline void error(const char* message) {
    Serial.print("[ERROR] ");
    Serial.println(message);
}

inline void infof(const char* fmt, ...) {
    Serial.print("[INFO] ");
    va_list args;
    va_start(args, fmt);
    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    Serial.println(buffer);
}

} // namespace Log
