#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using std::size_t;

#ifndef D1
#define D1 5
#define D2 4
#define D5 14
#endif

class MockSerial {
public:
    void print(const char* s) { (void)s; }
    void print(char c) { (void)c; }
    void println(const char* s) { (void)s; }
    void println() {}
    void begin(unsigned long) {}
};

inline MockSerial Serial;

class String {
public:
    String() = default;
    String(const char* s) : data_(s ? s : "") {}
    String(const std::string& s) : data_(s) {}

    size_t length() const { return data_.size(); }
    char operator[](size_t i) const { return data_[i]; }

    String substring(size_t start, size_t end = static_cast<size_t>(-1)) const {
        if (start >= data_.size()) {
            return String("");
        }
        if (end == static_cast<size_t>(-1) || end > data_.size()) {
            end = data_.size();
        }
        return String(data_.substr(start, end - start));
    }

    void trim() {
        const size_t start = data_.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            data_.clear();
            return;
        }
        const size_t end = data_.find_last_not_of(" \t\r\n");
        data_ = data_.substr(start, end - start + 1);
    }

    const char* c_str() const { return data_.c_str(); }

    void toCharArray(char* buffer, size_t size) const {
        if (!buffer || size == 0) {
            return;
        }
        std::snprintf(buffer, size, "%s", data_.c_str());
    }

    String operator+(const String& other) const { return String(data_ + other.data_); }
    String& operator=(const char* s) {
        data_ = s ? s : "";
        return *this;
    }

    bool operator==(const char* s) const { return data_ == (s ? s : ""); }

private:
    std::string data_;
};

inline String operator+(const char* lhs, const String& rhs) {
    return String(std::string(lhs ? lhs : "") + std::string(rhs.c_str()));
}

class ESPClass {
public:
    uint32_t getChipId() const { return chipId_; }
    void setChipId(uint32_t id) { chipId_ = id; }

private:
    uint32_t chipId_ = 123456;
};

inline ESPClass ESP;
