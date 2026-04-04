#pragma once
#include "NetworkClient.h"
#include "../../../model/Context/UEContext.h"

class UEExchange {
    AppSettings& settings;
    NetworkClient api;
    int enodebId=-1;
    bool IN_ACTIVE = false;

    json handleAuth();
    json handleAttachResponse();
    // void handleSMS(const json& res);

public:
    json radioMeasure();

    void attach();

    // void sendSMS(const std::string& msisdn, const std::string& text);

    void connect();

    bool isConnected() const { return IN_ACTIVE; };
    UEExchange(AppSettings& settings_);
};

