#pragma once

#include "../include/Transport.h"
#include "../../ENODE/include/ENodeB.h"
#include "Sender.h"

struct ClientState {
    std::vector<char> buffer;
    uint32_t expected_len = 0;
    bool reading_len = true;
};

class Gateway : public Transport, public Sender{

    std::unordered_map<int, ClientState> clients;
    std::unordered_map<std::string, int> tmsiToFd;

    std::unordered_map<int, ENodeB*>& ENodes;

    json handleManyNodes(json& req) const;

    json handleNode(json& req, int id) const;

    json packageProcess(const char *buf) const;

    void onAccept(OpContext *ctx, int res) override;
    void onRecv(OpContext *ctx, int res) override;
    void onSend(const OpContext *ctx, int res) override;
    void sendJSON(int fd, const json& msg);
public:
    Gateway(int PORT_, std::unordered_map<int, ENodeB*>& ENodes_): Transport(PORT_), ENodes(ENodes_){};
    void sendByTMSI(const std::string &tmsi, const json &msg) override;
};
