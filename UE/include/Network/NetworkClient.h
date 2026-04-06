#pragma once

#include <nlohmann/json.hpp>
#include "spdlog/spdlog.h"
#include "../../include/utils/utils.h"

using json = nlohmann::json;

class NetworkClient {
    int fd=-1;
    std::mutex lock;
    std::thread ping_thread;
    std::atomic<bool> connectionFlag=false;
    static constexpr size_t BUF_SIZE = 1024;
    NetworkAddress address;

    void ping();

public:
    int createConnection();

    void sendJSON(const json& j) const;
    json recvJSON();

    [[nodiscard]] bool isConnected() const {return connectionFlag;}

    explicit NetworkClient(const NetworkAddress& address_);
    ~NetworkClient();
};
