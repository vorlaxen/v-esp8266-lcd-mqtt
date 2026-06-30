#include "display/DisplayModule.h"
#include "display/TurkishLcdText.h"
#include "core/UiStrings.h"
#include "core/DeviceConstants.h"
#include <Wire.h>

DisplayModule::DisplayModule(EventBus& eventBus) : eventBus_(eventBus) {}

void DisplayModule::init() {
    Wire.begin(Device::kLcdSdaPin, Device::kLcdSclPin);
    lcd_.init();
    lcd_.backlight();
    showLines(Ui::kBootSplashLine1, Ui::kBootSplashLine2);

    eventBus_.subscribe(EventType::DisplayStatus,
                        [this](const Event& event) { onDisplayStatus(event); });
    eventBus_.subscribe(EventType::NetworkConnected,
                        [this](const Event& event) { onNetworkConnected(event); });
    eventBus_.subscribe(EventType::NetworkDisconnected,
                        [this](const Event& event) { onNetworkDisconnected(event); });
    eventBus_.subscribe(EventType::DisplayMessageRequested,
                        [this](const Event& event) { onDisplayMessage(event); });
    eventBus_.subscribe(EventType::DisplayRestoreNetwork,
                        [this](const Event& event) { onRestoreNetwork(event); });
}

void DisplayModule::loop() {
    if (mode_ == Mode::Message && scrolling_) {
        updateScroll();
    }
}

void DisplayModule::showLines(const String& line1, const String& line2) {
    lcd_.clear();
    TurkishLcdText::printLine(lcd_, 0, line1, Device::kLcdCols);
    TurkishLcdText::printLine(lcd_, 1, line2, Device::kLcdCols);
}

void DisplayModule::showNetworkInfo() {
    if (ip_.length() > 0) {
        mode_ = Mode::Network;
        scrolling_ = false;
        showLines(ip_, hostname_);
        return;
    }

    showLines(Ui::kNoConnection, Ui::kNoConnectionLine2);
}

void DisplayModule::renderMessage() {
    scrolling_ = false;
    scrollCharOffset_ = 0;

    const size_t charLen = TurkishLcdText::charCount(message_);
    if (charLen == 0) {
        showLines("", "");
        return;
    }

    if (charLen <= Device::kLcdCols) {
        showLines(message_, "");
        return;
    }

    if (charLen <= Device::kLcdCols * Device::kLcdRows) {
        showLines(TurkishLcdText::substringChars(message_, 0, Device::kLcdCols),
                  TurkishLcdText::substringChars(message_, Device::kLcdCols, Device::kLcdCols));
        return;
    }

    if (scrollEnabled_) {
        scrolling_ = true;
        lastScrollMs_ = 0;
        updateScroll();
        return;
    }

    showLines(TurkishLcdText::substringChars(message_, 0, Device::kLcdCols),
              TurkishLcdText::substringChars(message_, Device::kLcdCols, Device::kLcdCols));
}

void DisplayModule::updateScroll() {
    const String padded = String("  ") + message_ + String("  ");
    const size_t paddedLen = TurkishLcdText::charCount(padded);

    if (millis() - lastScrollMs_ < scrollDelayMs_) {
        return;
    }

    lastScrollMs_ = millis();

    if (scrollCharOffset_ + Device::kLcdCols >= paddedLen) {
        scrollCharOffset_ = 0;
    } else {
        scrollCharOffset_++;
    }

    const String line1 =
        TurkishLcdText::substringChars(padded, scrollCharOffset_, Device::kLcdCols);
    showLines(line1, "");
}

void DisplayModule::onDisplayStatus(const Event& event) {
    mode_ = Mode::Status;
    scrolling_ = false;
    showLines(event.line1, event.line2);
}

void DisplayModule::onNetworkConnected(const Event& event) {
    ip_ = event.ip;
    hostname_ = event.hostname;

    if (mode_ != Mode::Message) {
        showNetworkInfo();
    }
}

void DisplayModule::onNetworkDisconnected(const Event& event) {
    (void)event;
    ip_ = "";
    hostname_ = "";
    mode_ = Mode::Status;
    scrolling_ = false;
    showLines(Ui::kConnectionLost, Ui::kRetrying);
}

void DisplayModule::onDisplayMessage(const Event& event) {
    message_ = event.message;
    if (message_.length() > Device::kMaxMessageLen) {
        message_ = message_.substring(0, Device::kMaxMessageLen);
    }

    scrollEnabled_ = event.scrollEnabled;
    scrollDelayMs_ = event.scrollDelayMs > Device::kMinScrollDelayMs
                         ? event.scrollDelayMs
                         : Device::kDefaultScrollDelayMs;
    mode_ = Mode::Message;
    renderMessage();
}

void DisplayModule::onRestoreNetwork(const Event& event) {
    (void)event;
    message_ = "";
    scrolling_ = false;
    showNetworkInfo();
}
