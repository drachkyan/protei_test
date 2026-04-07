#pragma once

#include <string>
#include "spdlog/spdlog.h"
#include "../SMS/SMSRecord.h"

struct Chat {
    std::vector<SMSRecord> incoming;
    std::map<int, SMSRecord> outgoing;
};

class Context {

    const std::string IMSI;
    const std::string IMEI;
    const std::string MSISDN;
    std::string TMSI{};
    int x = 0;
    int sentSmsCounter = 0;

    std::unordered_map<std::string, Chat> chats;

public:
    [[nodiscard]] std::string getIMSI() const {return IMSI;}
    [[nodiscard]] std::string getIMEI() const {return IMEI;}
    [[nodiscard]] std::string getMSISDN() const {return MSISDN;}
    [[nodiscard]] std::string getTMSI() const {return TMSI;}

    void move(int x_) { x = x_;}
    int getX() const {return x;}
    void setTMSI(std::string TMSI_);
    void clearTMSI() {TMSI = "";}

    Context(std::string IMSI, std::string IMEI, std::string MSISDN, int x_);

    Chat* getChat(std::string& msisdn);
    int addSMS(const std::string &msisdn, const SMSRecord &SMS_);
    void receiveSMS(const std::string &msisdn, const SMSRecord &SMS_);
    void changeSMSStatus(const std::string &msisdn, int id, MessageStatus status_);
};

