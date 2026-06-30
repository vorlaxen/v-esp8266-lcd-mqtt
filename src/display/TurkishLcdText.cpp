#include "display/TurkishLcdText.h"

namespace {

struct GlyphEntry {
    uint32_t codepoint;
    uint8_t pattern[8];
    char fallback;
};

constexpr GlyphEntry kGlyphs[] = {
    {0x00E7, {0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x04}, 'c'}, // ç
    {0x011F, {0x08, 0x08, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00}, 'g'}, // ğ
    {0x0131, {0x00, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x00, 0x00}, 'i'}, // ı = i govdesi (nokta ve taban yok)
    {0x00F6, {0x00, 0x0A, 0x00, 0x0E, 0x11, 0x11, 0x0E, 0x00}, 'o'}, // ö
    {0x015F, {0x00, 0x00, 0x0E, 0x11, 0x0E, 0x01, 0x0E, 0x04}, 's'}, // ş
    {0x00FC, {0x00, 0x0A, 0x00, 0x11, 0x11, 0x11, 0x0E, 0x00}, 'u'}, // ü
    {0x00C7, {0x04, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x04}, 'C'}, // Ç
    {0x011E, {0x04, 0x0A, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00}, 'G'}, // Ğ
    {0x0130, {0x04, 0x00, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00}, 'I'}, // İ
    {0x00D6, {0x04, 0x0A, 0x00, 0x0E, 0x11, 0x11, 0x0E, 0x00}, 'O'}, // Ö
    {0x015E, {0x04, 0x00, 0x0E, 0x11, 0x0E, 0x01, 0x0E, 0x04}, 'S'}, // Ş
    {0x00DC, {0x04, 0x0A, 0x00, 0x11, 0x11, 0x11, 0x0E, 0x00}, 'U'}, // Ü
};

constexpr size_t kGlyphCount = sizeof(kGlyphs) / sizeof(kGlyphs[0]);
constexpr uint8_t kFirstSlot = 1;
constexpr uint8_t kLastSlot = 7;
constexpr uint8_t kSlotCount = 7;
constexpr uint32_t kEmptySlot = 0xFFFFFFFFUL;

bool decodeUtf8(const String& text, size_t byteIndex, uint32_t& codepoint, size_t& byteLength) {
    if (byteIndex >= static_cast<size_t>(text.length())) {
        return false;
    }

    const uint8_t first = static_cast<uint8_t>(text[byteIndex]);

    if ((first & 0x80) == 0) {
        codepoint = first;
        byteLength = 1;
        return true;
    }

    if ((first & 0xE0) == 0xC0 && byteIndex + 1 < static_cast<size_t>(text.length())) {
        const uint8_t second = static_cast<uint8_t>(text[byteIndex + 1]);
        codepoint = ((first & 0x1F) << 6) | (second & 0x3F);
        byteLength = 2;
        return true;
    }

    if ((first & 0xF0) == 0xE0 && byteIndex + 2 < static_cast<size_t>(text.length())) {
        const uint8_t second = static_cast<uint8_t>(text[byteIndex + 1]);
        const uint8_t third = static_cast<uint8_t>(text[byteIndex + 2]);
        codepoint = ((first & 0x0F) << 12) | ((second & 0x3F) << 6) | (third & 0x3F);
        byteLength = 3;
        return true;
    }

    codepoint = '?';
    byteLength = 1;
    return true;
}

const GlyphEntry* findGlyph(uint32_t codepoint) {
    for (size_t i = 0; i < kGlyphCount; i++) {
        if (kGlyphs[i].codepoint == codepoint) {
            return &kGlyphs[i];
        }
    }
    return nullptr;
}

int findSlotForCodepoint(uint32_t codepoint, const uint32_t* slotCodepoint) {
    for (uint8_t slot = kFirstSlot; slot <= kLastSlot; slot++) {
        if (slotCodepoint[slot] == codepoint) {
            return slot;
        }
    }
    return -1;
}

int allocateSlot(uint32_t codepoint, uint32_t* slotCodepoint, uint8_t& nextFreeSlot, uint8_t& evictSlot) {
    const int existing = findSlotForCodepoint(codepoint, slotCodepoint);
    if (existing >= 0) {
        return existing;
    }

    if (nextFreeSlot <= kLastSlot) {
        const int slot = nextFreeSlot++;
        slotCodepoint[slot] = codepoint;
        return slot;
    }

    const int slot = evictSlot;
    evictSlot = static_cast<uint8_t>(evictSlot >= kLastSlot ? kFirstSlot : evictSlot + 1);
    slotCodepoint[slot] = codepoint;
    return slot;
}

} // namespace

size_t TurkishLcdText::charCount(const String& text) {
    size_t count = 0;
    size_t index = 0;

    while (index < static_cast<size_t>(text.length())) {
        uint32_t codepoint = 0;
        size_t byteLength = 0;
        decodeUtf8(text, index, codepoint, byteLength);
        index += byteLength;
        count++;
    }

    return count;
}

String TurkishLcdText::substringChars(const String& text, size_t startChar, size_t charCount) {
    size_t currentChar = 0;
    size_t startByte = 0;
    size_t endByte = static_cast<size_t>(text.length());
    size_t index = 0;

    while (index < static_cast<size_t>(text.length())) {
        uint32_t codepoint = 0;
        size_t byteLength = 0;
        decodeUtf8(text, index, codepoint, byteLength);

        if (currentChar == startChar) {
            startByte = index;
        }

        if (currentChar == startChar + charCount) {
            endByte = index;
            break;
        }

        index += byteLength;
        currentChar++;
    }

    return text.substring(startByte, endByte);
}

void TurkishLcdText::printLine(LiquidCrystal_I2C& lcd, uint8_t row, const String& text, uint8_t maxCols) {
    uint32_t codepoints[16];
    uint8_t slots[16];
    size_t charLength = 0;
    size_t index = 0;

    while (index < static_cast<size_t>(text.length()) && charLength < maxCols) {
        uint32_t codepoint = 0;
        size_t byteLength = 0;
        decodeUtf8(text, index, codepoint, byteLength);
        codepoints[charLength++] = codepoint;
        index += byteLength;
    }

    uint32_t slotCodepoint[8];
    for (uint8_t i = 0; i < 8; i++) {
        slotCodepoint[i] = kEmptySlot;
    }

    uint8_t nextFreeSlot = kFirstSlot;
    uint8_t evictSlot = kFirstSlot;

    for (size_t i = 0; i < charLength; i++) {
        slots[i] = 0;

        if (codepoints[i] < 0x80) {
            continue;
        }

        const GlyphEntry* glyph = findGlyph(codepoints[i]);
        if (!glyph) {
            continue;
        }

        const int slot = allocateSlot(codepoints[i], slotCodepoint, nextFreeSlot, evictSlot);
        slots[i] = static_cast<uint8_t>(slot);
        lcd.createChar(static_cast<uint8_t>(slot), const_cast<uint8_t*>(glyph->pattern));
    }

    for (size_t col = 0; col < charLength; col++) {
        lcd.setCursor(col, row);

        const uint32_t codepoint = codepoints[col];
        if (codepoint < 0x80) {
            lcd.write(static_cast<uint8_t>(codepoint));
            continue;
        }

        if (slots[col] >= kFirstSlot) {
            lcd.write(slots[col]);
            continue;
        }

        const GlyphEntry* glyph = findGlyph(codepoint);
        lcd.write(static_cast<uint8_t>(glyph ? glyph->fallback : '?'));
    }

    for (size_t col = charLength; col < maxCols; col++) {
        lcd.setCursor(col, row);
        lcd.write(' ');
    }
}
