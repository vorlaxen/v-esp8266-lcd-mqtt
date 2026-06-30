#pragma once

#include "core/IModule.h"
#include "core/EventBus.h"
#include "core/DeviceConstants.h"
#include <LiquidCrystal_I2C.h>

class DisplayModule : public IModule {
public:
    explicit DisplayModule(EventBus& eventBus);

    void init() override;
    void loop() override;

private:
    enum class Mode {
        Status,
        Network,
        Message
    };

    EventBus& eventBus_;
    LiquidCrystal_I2C lcd_{Device::kLcdAddress, Device::kLcdCols, Device::kLcdRows};

    String ip_;
    String hostname_;
    String message_;
    Mode mode_ = Mode::Status;
    bool scrolling_ = false;
    bool scrollEnabled_ = true;
    size_t scrollCharOffset_ = 0;
    uint16_t scrollDelayMs_ = Device::kDefaultScrollDelayMs;
    unsigned long lastScrollMs_ = 0;

    void showLines(const String& line1, const String& line2);
    void renderMessage();
    void updateScroll();
    void showNetworkInfo();

    void onDisplayStatus(const Event& event);
    void onNetworkConnected(const Event& event);
    void onNetworkDisconnected(const Event& event);
    void onDisplayMessage(const Event& event);
    void onRestoreNetwork(const Event& event);
};
