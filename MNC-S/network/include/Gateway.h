#pragma once

#include "../include/Transport.h"
#include "../include/RequestHandler.h"
#include "../../ENODE/include/ENodeB.h"

struct ClientState {
    std::vector<char> buffer;
    uint32_t expected_len = 0;
    bool reading_len = true;
};

class Gateway : public Transport{

    std::unordered_map<int, ClientState> clients;

    std::unordered_map<int, ENodeB*>& ENodes;

    json handleManyNodes(json& req) const;

    json handleNode(json& req, int id) const;

    json packageProcess(const char *buf) const {
        json req = json::parse(buf);

        json res = !req.contains("ENode") ?
            handleManyNodes(req) :
            handleNode(req, req["ENode"].get<int>());

        return res;
    }

    void onAccept(OpContext *ctx, int res) override;
    void onRecv(OpContext *ctx, int res) override;
    void onSend(const OpContext *ctx, int res) override;
public:
    Gateway(int PORT_, std::unordered_map<int, ENodeB*>& ENodes_): Transport(PORT_), ENodes(ENodes_){};
};
