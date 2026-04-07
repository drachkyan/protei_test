#pragma once

#include "../include/Listener.h"
#include "../../ENODE/include/ENodeB.h"
#include "Sender.h"

struct ClientState {
    std::vector<char> buffer;
    int enodeID = -1;
    uint32_t expected_len = 0;
    bool reading_len = true;
    std::string tmsi{};
};

class UEconnection : public Listener, public Sender{

    std::unordered_map<int, ClientState> clients;
    std::unordered_map<std::string, int> tmsiToFd;

    std::unordered_map<int, ENodeB*>& ENodes;
    std::mutex clientsMtx;
    json handleManyNodes(json& req) const;

    json handleNode(json& req, int id) const;

    json packageProcess(const char *buf, int client_fd);

    void onAccept(OpContext *ctx, int res) override;
    void onRecv(OpContext *ctx, int res) override;
    void onSend(const OpContext *ctx, int res) override;
    void sendJSON(int fd, const json& msg);
public:
    UEconnection(int PORT_, std::unordered_map<int, ENodeB*>& ENodes_): Listener(PORT_), ENodes(ENodes_){};
    void sendByTMSI(const std::string &tmsi, const json &msg) override;
};
