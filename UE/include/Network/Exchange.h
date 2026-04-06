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
    std::thread signalThread;
    std::atomic<bool> IN_ACTIVE = false;

    std::unordered_map<int, std::promise<json>> pending;
    std::atomic<int> requestId = 0;

    std::condition_variable signalCv;
    std::mutex pendingMtx;
    std::mutex signalMtx;

    json handleAttachRequest();
    json handleAuthResponse();

    void signalWorker();

    void onDisconnect();
public:
    void run();

    void shutdown() { IN_ACTIVE = false;};

    json sendAndWait(json& req);
    void send(json& req);
    json radioMeasure();


    void sendSMS(const std::string& msisdn, const std::string& msg);
    void sendSMSStatus(const std::string& msisdn_d, int id, MessageStatus status);

    void attach();
    void connect();

    bool isConnected() const { return IN_ACTIVE; };
    Exchange(AppSettings& settings_);
    ~Exchange();
};

