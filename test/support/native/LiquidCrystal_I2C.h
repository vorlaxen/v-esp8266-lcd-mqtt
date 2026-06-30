#pragma once

#include <cstdint>
#include <vector>
#include "Arduino.h"

struct LcdWriteCall {
    uint8_t row;
    uint8_t col;
    uint8_t value;
    bool customChar;
};

class LiquidCrystal_I2C {
public:
    explicit LiquidCrystal_I2C(uint8_t, uint8_t, uint8_t) {}

    void createChar(uint8_t slot, uint8_t* pattern) {
        (void)pattern;
        lastCustomSlot_ = slot;
    }

    void setCursor(uint8_t col, uint8_t row) {
        cursorCol_ = col;
        cursorRow_ = row;
    }

    void write(uint8_t value) {
        writes_.push_back({cursorRow_, cursorCol_, value, value >= 1 && value <= 7});
    }

    void clear() { writes_.clear(); }

    const std::vector<LcdWriteCall>& writes() const { return writes_; }
    uint8_t lastCustomSlot() const { return lastCustomSlot_; }

private:
    uint8_t cursorCol_ = 0;
    uint8_t cursorRow_ = 0;
    uint8_t lastCustomSlot_ = 0;
    std::vector<LcdWriteCall> writes_;
};
