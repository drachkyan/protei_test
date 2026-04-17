#pragma once

#include "../Network/NetworkAddress.h"
#include "../../../model/Context/Context.h"

class AppSettings {
    NetworkAddress netAddr;
    Context context;


public:
    AppSettings(NetworkAddress netAddr_, Context context_)
        : netAddr(std::move(netAddr_)), context(std::move(context_)) {}

    NetworkAddress& getNetworkAddress() { return netAddr; }
    Context& getContext() { return context; }
};