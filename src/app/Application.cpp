#include "app/Application.h"
#include "core/DeviceConstants.h"
#include "core/Log.h"
#include <LittleFS.h>

Application::Application()
    : display_(eventBus_),
      network_(config_, eventBus_),
      mqtt_(config_, network_, eventBus_),
      webServer_(config_, network_, mqtt_, eventBus_),
      factoryReset_(config_, network_, eventBus_) {
    modules_[0] = &factoryReset_;
    modules_[1] = &network_;
    modules_[2] = &mqtt_;
    modules_[3] = &webServer_;
    modules_[4] = &display_;
}

void Application::setup() {
    Serial.begin(Device::kSerialBaud);
    delay(Device::kBootDelayMs);

    while (Serial.available() > 0) {
        Serial.read();
    }

    Serial.println();
    Serial.println("----- Vorlaxen -----");
    Log::info("Vorlaxen boot...");

    if (!LittleFS.begin()) {
        Log::error("LittleFS mount failed");
    } else {
        Log::info("LittleFS OK");
    }

    initModules();
    Log::info("Ready");
}

void Application::initModules() {
    config_.init();
    display_.init();
    webServer_.init();
    mqtt_.init();
    factoryReset_.init();
    network_.init();
}

void Application::loop() {
    for (size_t i = 0; i < kModuleCount; i++) {
        modules_[i]->loop();
    }
}
