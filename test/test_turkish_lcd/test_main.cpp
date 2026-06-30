#include <unity.h>
#include "display/TurkishLcdText.h"
#include "LiquidCrystal_I2C.h"

void test_char_count_ascii() {
    TEST_ASSERT_EQUAL(5, TurkishLcdText::charCount(String("Hello")));
}

void test_char_count_turkish_utf8() {
    TEST_ASSERT_EQUAL(7, TurkishLcdText::charCount(String("Merhaba")));
    TEST_ASSERT_EQUAL(8, TurkishLcdText::charCount(String("Merhaba!")));
    TEST_ASSERT_EQUAL(2, TurkishLcdText::charCount(String("çğ")));
}

void test_substring_chars_ascii() {
    const String result = TurkishLcdText::substringChars(String("Hello World"), 0, 5);
    TEST_ASSERT_EQUAL_STRING("Hello", result.c_str());
}

void test_substring_chars_utf8() {
    const String text = String("İstanbul");
    TEST_ASSERT_EQUAL(8, TurkishLcdText::charCount(text));
    const String slice = TurkishLcdText::substringChars(text, 0, 3);
    TEST_ASSERT_EQUAL(4, slice.length());
    TEST_ASSERT_EQUAL_STRING("İst", slice.c_str());
}

void test_print_line_ascii_only() {
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    TurkishLcdText::printLine(lcd, 0, String("Hello"), 16);
    TEST_ASSERT_GREATER_THAN(0, lcd.writes().size());
    TEST_ASSERT_EQUAL('H', lcd.writes().front().value);
}

void test_print_line_turkish_uses_custom_slots() {
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    TurkishLcdText::printLine(lcd, 0, String("çğ"), 16);
    bool usedCustom = false;
    for (const LcdWriteCall& call : lcd.writes()) {
        if (call.customChar) {
            usedCustom = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(usedCustom);
}

void test_print_line_pads_to_max_cols() {
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    TurkishLcdText::printLine(lcd, 0, String("Hi"), 16);
    TEST_ASSERT_EQUAL(16, lcd.writes().size());
}

void setUp() {}
void tearDown() {}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_char_count_ascii);
    RUN_TEST(test_char_count_turkish_utf8);
    RUN_TEST(test_substring_chars_ascii);
    RUN_TEST(test_substring_chars_utf8);
    RUN_TEST(test_print_line_ascii_only);
    RUN_TEST(test_print_line_turkish_uses_custom_slots);
    RUN_TEST(test_print_line_pads_to_max_cols);
    return UNITY_END();
}
