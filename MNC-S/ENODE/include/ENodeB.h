#pragma once
#include <array>
#include <queue>
#include <future>
#include <nlohmann/json.hpp>
#include <memory>

#include "../../ENODE/include/EnodeHandler.h"
#include "../../network/include/RequestHandler.h"
#include "../../../model/ENode/EnodeConfig.h"
#include "../../MME/include/MME.h"

using json = nlohmann::json;

struct Task {
    json data;
    std::promise<json> promise;
};

struct Slot {
    std::queue<std::string> inbox;
    std::queue<std::string> outbox;
};


class ENodeB {
    MME& mme;
    ENodeConfig config;
    std::unique_ptr<EnodeHandler> worker;

    std::mutex taskMtx;
    std::condition_variable taskCond;

    std::atomic<bool> stop{false};
    static constexpr int MAX_CONNECTIONS = 4;

    std::queue<Task> tasks;
    std::mutex slotMtx;

    std::unordered_map<std::string, Slot> slots;

    bool hasFreeSlot() const;
public:
    ENodeB(int id_, double x_, double power_, double radius_, MME& mme_);
    void run();
    void shutdown();
    void push(Task msg);

    bool reserveSlot(const std::string& tmsi);
    void releaseSlot(const std::string &tmsi);
};

