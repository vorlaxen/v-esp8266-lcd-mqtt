#pragma once

class IProvisioningResettable {
public:
    virtual ~IProvisioningResettable() = default;
    virtual void resetToProvisioning() = 0;
};
