#pragma once

#include "core/IModule.h"
#include "config/ConfigModule.h"
#include "network/IProvisioningResettable.h"
#include "core/EventBus.h"
#include "core/DeviceConstants.h"

class FactoryResetModule : public IModule {
public:
    FactoryResetModule(ConfigModule& config,
                       IProvisioningResettable& provisioning,
                       EventBus& eventBus);

    void init() override;
    void loop() override;

private:
    ConfigModule& config_;
    IProvisioningResettable& provisioning_;
    EventBus& eventBus_;

    bool resetDoneThisPress_ = false;
    bool resetPending_ = false;
    unsigned long pressedSince_ = 0;
    unsigned long resetProvisioningAt_ = 0;
    unsigned long bootAt_ = 0;

    void performReset();
};
