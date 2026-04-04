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
    static const int MAX_CONNECTIONS = 4;

    std::queue<Task> tasks;

    std::array<Slot, MAX_CONNECTIONS> slots;
    std::unordered_map<std::string, int> tmsiToSlot;

public:
    ENodeB(int id_, double x_, double power_, double radius_, MME& mme_);
    void run();
    void shutdown();
    void push(Task msg);
};

