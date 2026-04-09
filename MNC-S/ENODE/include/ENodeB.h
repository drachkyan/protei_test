#pragma once

#include <queue>
#include <future>
#include <nlohmann/json.hpp>
#include <memory>

#include "../../ENODE/include/EnodeHandler.h"
#include "../../../model/Handler/RequestHandler.h"
#include "../../../model/ENode/EnodeConfig.h"
#include "../../MME/include/MME.h"

using json = nlohmann::json;

struct Task {
    json data;
    std::promise<json> promise;
};

struct SMSMessage {
    int sms_id;
    std::string text;
    std::string msisdn_src{};
    std::string tmsi_dst{};
};

// макрос создает to_json и from_json для смс меседж
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SMSMessage, sms_id, text, msisdn_src, tmsi_dst)

struct Slot {
    std::queue<SMSMessage> inbox;
    std::unordered_map<std::string, std::queue<SMSMessage>> outbox;   // msisdn - SMS queue
};


class ENodeB {
    MME& mme;

    ENodeConfig config;
    std::unique_ptr<EnodeHandler> worker;

    std::condition_variable taskCond;

    std::atomic<bool> stop{false};
    static constexpr int MAX_CONNECTIONS = 4;

    std::queue<Task> tasks;
    std::queue<Task> internalTasks;
    std::mutex taskMtx;

    std::mutex slotMtx;
    std::unordered_map<std::string, Slot> TMSItoSlots;

    bool hasFreeSlot() const;
public:
    ENodeB(int id_, double x_, double power_, double radius_,
        MME& mme_, std::unordered_map<int, ENodeB*>& ENodes_, Sender& sender_);

    void run();
    void shutdown();
    void push(Task msg);
    void pushInternalTask(Task msg);
    bool reserveSlot(const std::string& tmsi);
    void releaseSlot(const std::string& tmsi);

    void handover(std::string tmsi, Slot slot);

    std::optional<Slot> detachSlot(const std::string &tmsi);

    void addSMStoSlot(const std::string &tmsi_s, const std::string &msisdn_d, SMSMessage &msg);

    bool deleteSMSfromSlot(const std::string &tmsi_s, const std::string &msisdn_d);
    void receiveSMS(SMSMessage msg);

    std::optional<SMSMessage> getSMStoSend(const std::string& tmsi_s, const std::string& msisdn_d);
    SMSMessage getSMSbyTMSI(const std::string& tmsi);
};

