#pragma once

#include "../../../model/Handler/RequestHandler.h"
#include "../../XLR/include/XLR.h"


class ENodeB;

class MMEHandler final : public RequestHandler {
    using HandlerPtr = json (MMEHandler::*)(const json&);


    std::unordered_map<std::string, HandlerPtr> handlersMap;

    XLR& xlr;
    std::string TMSI_script;
    std::unordered_map<int, ENodeB*>& ENodes;

    void initHandlersMap();

    json handleAttach(const json& req);

    json handleUpdateLocation(const json& req);

    static std::string generateTMSI(std::string& scriptPath);

    json sendSMS(ENodeB *ENodeS, int enodeD, std::string TMSI_D, std::string TMSI_S, std::string MSISDN_S, std::string MSISDN_D);

    std::optional<Subscriber> handleWaitAbonent(std::string MSISDN_D);

    json handleSMS(const json& req);
    json handleSMSStatus(const json& req);
    json handleHandover(const json& req);
    json handleDisconnect(const json& req);
public:
    MMEHandler(std::string path, XLR& xlr_, std::unordered_map<int, ENodeB*>& ENodes_);
    json handle(const json& req) override;
    ~MMEHandler() override = default;
};


