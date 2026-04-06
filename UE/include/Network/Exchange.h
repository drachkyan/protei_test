#pragma once
#include <future>

#include "ExchangeHandler.h"
#include "NetworkClient.h"

class Exchange {
    AppSettings& settings;
    NetworkClient api;
    ExchangeHandler worker;

    int enodebId=-1;

    std::thread runThread;

    std::atomic<bool> IN_ACTIVE = false;

    std::unordered_map<int, std::promise<json>> pending;
    std::atomic<int> requestId = 0;

    std::mutex pendingMtx;

    json handleAttachRequest();
    json handleAuthResponse();

public:
    void run();

    void shutdown() { IN_ACTIVE = false;};

    json sendAndWait(json& req);
    json radioMeasure();

    void attach();

    void sendSMS(const std::string& msisdn, const std::string& msg);

    void connect();

    bool isConnected() const { return IN_ACTIVE; };
    Exchange(AppSettings& settings_);
    ~Exchange();
};

