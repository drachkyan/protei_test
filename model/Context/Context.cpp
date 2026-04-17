#include "Context.h"

Chat* Context::getChat(std::string &msisdn) {
    auto chat = chats.find(msisdn);
    if (chats.end() == chat) {
        return nullptr;
    }
    return &chats[msisdn];
}

int Context::addSMS(const std::string &msisdn, const SMSRecord &SMS_) {
    sentSmsCounter++;
    if (!chats.contains(msisdn)) {
        chats[msisdn] = Chat{};
    }
    chats[msisdn].outgoing.emplace(sentSmsCounter, SMS_);
    return sentSmsCounter;
}

void Context::receiveSMS(const std::string &msisdn, const SMSRecord &SMS_) {
    if (!chats.contains(msisdn)) {
        chats[msisdn] = Chat{};
    }
    chats[msisdn].incoming.push_back(SMS_);
}

void Context::changeSMSStatus(const std::string &msisdn, int id, MessageStatus status_) {\

    if (!chats.contains(msisdn)) {
        return;
    }
    auto& msg = chats[msisdn].outgoing[id];
    msg.status = status_;
}

void Context::setTMSI(std::string TMSI_) {
    TMSI = std::move(TMSI_);
}

Context::Context(std::string IMSI, std::string IMEI, std::string MSISDN, int x_):
            IMSI(std::move(IMSI)), IMEI(std::move(IMEI)), MSISDN(std::move(MSISDN)), x(x_) {}

