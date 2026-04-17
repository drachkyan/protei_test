#pragma once

#include "../../../model/Handler/RequestHandler.h"
#include "../Settings/AppSettings.h"

class Exchange;

class ExchangeHandler final {
    AppSettings& settings;
    Exchange& UEex;
    using HandlerPtr = void (ExchangeHandler::*)(const json&);

    std::unordered_map<std::string, HandlerPtr> handlersMap;

    void handleIncomeSMS(const json& req);
    void handleSMSStatus(const json& req);

    void initHandlersMap();
public:
    ExchangeHandler(AppSettings& settings_, Exchange& UEex_) : settings(settings_), UEex(UEex_){ initHandlersMap(); }
    void handle(const json& req);

};


