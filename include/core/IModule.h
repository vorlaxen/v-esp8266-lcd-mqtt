#pragma once

class IModule {
public:
    virtual ~IModule() = default;
    virtual void init() = 0;
    virtual void loop() = 0;
};
