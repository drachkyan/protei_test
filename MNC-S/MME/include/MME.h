#pragma once

#include <future>
#include <queue>
#include <nlohmann/json.hpp>

#include "MMEHandler.h"
#include "../../../model/Handler/RequestHandler.h"
#include "../../XLR/include/XLR.h"

class ENodeB;

using json = nlohmann::json;
struct MMETask {
    json data;
    std::promise<json> promise;
};

class MME {
    XLR xlr; // сначала бд тк для воркера нужно бд

    const int MAX_THREADS = 4;
    std::unique_ptr<MMEHandler> worker;
    std::atomic<bool> stop = false;
    std::unordered_map<int, ENodeB*> ENodes;
    std::queue<MMETask> tasks;

    std::mutex taskMtx;
    std::mutex enodeMtx;

    std::condition_variable taskCond;

public:
    MME(const std::string& path);
    void add_ENode(int id, ENodeB* enode);
    void run();
    void shutdown();
    void push(MMETask task);
    std::unordered_map<int, ENodeB*>& getENodes(){ return ENodes; }
};

