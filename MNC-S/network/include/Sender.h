#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Sender {
public:
    virtual void sendByTMSI(const std::string& tmsi, const json& msg) = 0;
    virtual ~Sender() = default;
};