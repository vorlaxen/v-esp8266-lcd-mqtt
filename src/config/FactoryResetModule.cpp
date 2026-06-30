#include "config/FactoryResetModule.h"
#include "core/UiStrings.h"

FactoryResetModule::FactoryResetModule(ConfigModule& config,
                                       IProvisioningResettable& provisioning,
                                       EventBus& eventBus)
    : config_(config), provisioning_(provisioning), eventBus_(eventBus) {}

void FactoryResetModule::init() {
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
    bootAt_ = millis();
}

void FactoryResetModule::loop() {
    if (resetPending_ && millis() >= resetProvisioningAt_) {
        resetPending_ = false;
        provisioning_.resetToProvisioning();
    }

    if (millis() - bootAt_ < Device::kFactoryResetGraceMs) {
        return;
    }

    const bool pressed = digitalRead(FACTORY_RESET_PIN) == LOW;

    if (!pressed) {
        pressedSince_ = 0;
        resetDoneThisPress_ = false;
        return;
    }

    if (resetDoneThisPress_ || resetPending_) {
        return;
    }

    if (pressedSince_ == 0) {
        pressedSince_ = millis();
    }

    if (millis() - pressedSince_ >= Device::kFactoryResetHoldMs) {
        performReset();
        resetDoneThisPress_ = true;
    }
}

void FactoryResetModule::performReset() {
    config_.clearWiFi();
    eventBus_.publish(makeDisplayStatusEvent(Ui::kWifiCleared, SETUP_AP_SSID));
    resetPending_ = true;
    resetProvisioningAt_ = millis() + Device::kResetMessageHoldMs;
}
