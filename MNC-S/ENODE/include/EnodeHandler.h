#pragma once

#include "../../../model/Handler/RequestHandler.h"
#include "../../../model/ENode/EnodeConfig.h"
#include "../../MME/include/MME.h"
#include "../../network/include/Sender.h"


class EnodeHandler final
    : public RequestHandler
{
    using HandlerPtr = json (EnodeHandler::*)(const json&);


    std::unordered_map<std::string, HandlerPtr> handlersMap;
    std::unordered_map<std::string, HandlerPtr> handlersInternalMap;

    void initHandlersMap();

    ENodeConfig& config;
    MME& mme;
    Sender& sender;
    std::unordered_map<int, ENodeB*>& ENodes;
    json handleAuth(const json& req);

    json dispatch(const json &req, const std::unordered_map<std::string, HandlerPtr> &map);

    void proccessSMS(const json &req);

    json handleRadioMeasure(const json& req);
    json handleUpdateLocation(const json &req);
    json handleSendMessage(const json &req);
    json handleMessage(const json &req);
    json handleReceiveSMS(const json &req);
    json handleSendSMS(const json &req);
    json handleSMSStatus(const json &req);

    json handleDisconnect(const json &req);

    json requestMME(const json req) const;
public:
    EnodeHandler(ENodeConfig& config_, MME& mme_, std::unordered_map<int, ENodeB*>& ENodes_, Sender& sender_);

    [[nodiscard]] json handle(const json& req) override;
    [[nodiscard]] json handleInternalTask(const json &req);


    ~EnodeHandler() override = default;
};
