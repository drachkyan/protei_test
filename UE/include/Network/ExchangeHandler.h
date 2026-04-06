#pragma once

#include "../../../model/Handler/RequestHandler.h"
#include "../Settings/AppSettings.h"

class ExchangeHandler final {
    AppSettings& settings;
    using HandlerPtr = void (ExchangeHandler::*)(const json&);

    std::unordered_map<std::string, HandlerPtr> handlersMap;

    void handleIncomeSMS(const json& req);
    void initHandlersMap();

public:
    ExchangeHandler(AppSettings& settings_) : settings(settings_) { initHandlersMap(); }
    void handle(const json& req);

};


