#pragma once

#include "../../network/include/RequestHandler.h"
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

public:
    MMEHandler(std::string path, XLR& xlr_, std::unordered_map<int, ENodeB*>& ENodes_);
    json handle(const json& req) override;
    ~MMEHandler() override = default;
};


