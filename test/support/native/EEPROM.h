#pragma once

#include <cstring>
#include "Arduino.h"

class EEPROMClass {
public:
    bool begin(size_t size) {
        if (size_ == 0) {
            size_ = size;
            buffer_.assign(size, 0xFF);
        }
        return size_ >= size;
    }

    template <typename T>
    void get(size_t address, T& value) {
        std::memcpy(&value, buffer_.data() + address, sizeof(T));
    }

    template <typename T>
    void put(size_t address, const T& value) {
        std::memcpy(buffer_.data() + address, &value, sizeof(T));
    }

    bool commit() { return !commitFails_; }

    void reset() {
        size_ = 0;
        buffer_.clear();
        commitFails_ = false;
    }

    void setCommitFails(bool fails) { commitFails_ = fails; }

    const uint8_t* data() const { return buffer_.data(); }
    size_t size() const { return size_; }

private:
    size_t size_ = 0;
    std::vector<uint8_t> buffer_;
    bool commitFails_ = false;
};

inline EEPROMClass EEPROM;
