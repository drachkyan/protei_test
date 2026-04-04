#pragma once

#include "../../network/include/RequestHandler.h"
#include "../../XLR/include/XLR.h"

class MMEHandler final : public RequestHandler {
    using HandlerPtr = json (MMEHandler::*)(const json&);


    std::unordered_map<std::string, HandlerPtr> handlersMap;

    XLR& xlr;
    std::string TMSI_script;

    void initHandlersMap();

    json handleAttach(const json& req);

    json handleUpdateLocation(const json& req);

    static std::string generateTMSI(std::string& scriptPath);

public:
    MMEHandler(std::string path, XLR& xlr_);
    json handle(const json& req) override;
    ~MMEHandler() override = default;
};


