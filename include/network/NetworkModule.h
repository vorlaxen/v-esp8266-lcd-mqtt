#pragma once

#include "core/IModule.h"
#include "config/ConfigModule.h"
#include "network/IProvisioningResettable.h"
#include "network/WiFiProvisioner.h"
#include "core/EventBus.h"
#include <memory>

class NetworkModule : public IModule, public IProvisioningResettable {
public:
    NetworkModule(ConfigModule& config, EventBus& eventBus);

    void init() override;
    void loop() override;

    bool isConnected() const;
    String getIp() const;
    String getHostname() const;
    String getSsid() const;
    int32_t getRssi() const;

    void resetToProvisioning() override;

private:
    enum class State {
        Idle,
        Provisioning,
        Connecting,
        Connected,
        Disconnected
    };

    ConfigModule& config_;
    EventBus& eventBus_;
    State state_ = State::Idle;
    std::unique_ptr<WiFiProvisioner> provisioner_;
    unsigned long lastReconnectAttempt_ = 0;
    unsigned long connectStartedAt_ = 0;

    void startProvisioning();
    void startConnecting();
    void handleConnected();
    void handleDisconnected();
    void handleConnectFailed();
};
