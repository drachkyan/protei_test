
#include "../../include/Network/ExchangeHandler.h"
#include "../../include/Network/Exchange.h"
#include "spdlog/spdlog.h"

void ExchangeHandler::handleIncomeSMS(const json &req) {
    auto MSISDN = req["msisdn_src"].get<std::string>();
    auto SMS_ID = req["sms_id"].get<int>();
    auto text = req["text"].get<std::string>();
    spdlog::info("Пришло сообщение от {}: {}", MSISDN, text);
    UEex.sendSMSStatus(MSISDN, SMS_ID, MessageStatus::DELIVERED);
    SMSRecord SMS {MSISDN, text, MessageStatus::DELIVERED, std::chrono::system_clock::now()};
    settings.getContext().receiveSMS(MSISDN, SMS);
}

void ExchangeHandler::handleSMSStatus(const json &req) {
    if (!req.contains("message_status")) {
        spdlog::info("Пришла ошибка");
        return;
    }
    auto msisdn = req["MSISDN_D"].get<std::string>();
    int sms_id = req["SMS_ID"].get<int>();
    std::string status_str = req["message_status"].get<std::string>();

    auto status = stringToMessageStatus(status_str);
    settings.getContext().changeSMSStatus(msisdn, sms_id, status);

}

void ExchangeHandler::initHandlersMap() {
    handlersMap["SMS"] = &ExchangeHandler::handleIncomeSMS;
    handlersMap["SMSStatus"] = &ExchangeHandler::handleSMSStatus;

}

void ExchangeHandler::handle(const json &req) {

    if (!req.contains("type")) return;
    std::string type = req["type"].get<std::string>();

    auto it = handlersMap.find(type);
    if (it != handlersMap.end()) {
        (this->*it->second)(req);
    }
}
