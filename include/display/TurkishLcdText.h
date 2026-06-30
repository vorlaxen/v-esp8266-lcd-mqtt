#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class TurkishLcdText {
public:
    static size_t charCount(const String& text);
    static String substringChars(const String& text, size_t startChar, size_t charCount);
    static void printLine(LiquidCrystal_I2C& lcd, uint8_t row, const String& text, uint8_t maxCols);
};
