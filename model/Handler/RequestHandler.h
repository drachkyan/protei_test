#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class RequestHandler {

public:
    virtual json handle(const json& request) = 0;
    virtual ~RequestHandler() = default;
};
