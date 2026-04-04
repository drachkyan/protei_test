#pragma once

#include <string>
#include <utility>

#include "spdlog/spdlog.h"

class UEContext {
    const std::string IMSI;
    const std::string IMEI;
    const std::string MSISDN;
    std::string TMSI{};
    int x = 0;
public:
    [[nodiscard]] std::string getIMSI() const {return IMSI;}
    [[nodiscard]] std::string getIMEI() const {return IMEI;}
    [[nodiscard]] std::string getMSISDN() const {return MSISDN;}
    [[nodiscard]] std::string getTMSI() const {return TMSI;}

    void move(int x_){ x = x_;}
    int getX() const {return x;}
    void setTMSI(std::string TMSI_) {
        if (!TMSI.empty()) {
            spdlog::info("Ошибка - нельзя перезаписывать TMSI");
            return;
        }
        TMSI = std::move(TMSI_);
    }

    UEContext(std::string IMSI, std::string IMEI, std::string MSISDN, int x_):
            IMSI(std::move(IMSI)), IMEI(std::move(IMEI)), MSISDN(std::move(MSISDN)), x(x_) {};


};

