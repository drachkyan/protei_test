#pragma once

#include "../Network/NetworkAddress.h"
#include "../../../model/Context/UEContext.h"

class AppSettings {
    NetworkAddress netAddr;
    UEContext context;


public:
    AppSettings(NetworkAddress netAddr_, UEContext context_)
        : netAddr(std::move(netAddr_)), context(std::move(context_)) {}

    NetworkAddress& getNetworkAddress() { return netAddr; }
    UEContext& getContext() { return context; }
};