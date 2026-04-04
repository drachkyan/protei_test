#pragma once

#include "../../network/include/RequestHandler.h"
#include "../../../model/ENode/EnodeConfig.h"
#include "../../MME/include/MME.h"
class EnodeHandler final
    : public RequestHandler
{
    using HandlerPtr = json (EnodeHandler::*)(const json&);


    std::unordered_map<std::string, HandlerPtr> handlersMap;

    void initHandlersMap();

    ENodeConfig& config;
    MME& mme;

    json handleAuth(const json& req);
    json handleRadioMeasure(const json& req);
    json handleUpdateLocation(const json &req);

    json requestMME(const json& req) const;
public:
    EnodeHandler(ENodeConfig& config_m, MME& mme_);
    [[nodiscard]] json handle(const json& req) override;
    ~EnodeHandler() override = default;
};