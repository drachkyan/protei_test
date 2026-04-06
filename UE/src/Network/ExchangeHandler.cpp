
#include "../../include/Network/ExchangeHandler.h"

#include "spdlog/spdlog.h"

void ExchangeHandler::handleIncomeSMS(const json &req) {
    auto MSISDN = req["msisdn_src"].get<std::string>();
    auto text = req["text"].get<std::string>();
    spdlog::info("Пришло сообщение от {}: {}", MSISDN, text);
    SMSRecord SMS {MSISDN, text, MessageStatus::DELIVERED, std::chrono::system_clock::now()};
    settings.getContext().receiveSMS(MSISDN, SMS);
}

void ExchangeHandler::initHandlersMap() {
    handlersMap["SMS"] = &ExchangeHandler::handleIncomeSMS;

}

void ExchangeHandler::handle(const json &req) {

    if (!req.contains("type")) return;
    std::string type = req["type"].get<std::string>();

    auto it = handlersMap.find(type);
    if (it != handlersMap.end()) {
        (this->*it->second)(req);
    }
}
