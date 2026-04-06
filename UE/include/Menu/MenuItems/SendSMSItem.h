#pragma once

#include "../../Menu/MenuItem.h"
#include "../../Network/Exchange.h"

#include <iostream>

class SendSMSItem : public AbstractMenuItem {
    Exchange& UEex;
    Context& UEctx;
public:
    SendSMSItem(std::string name_, std::string description_, Exchange& UEex_, Context& ueCtx_):
        AbstractMenuItem(std::move(name_), std::move(description_)), UEex(UEex_), UEctx(ueCtx_)  {};


    MENU_EXITS action() override {
        if (!UEex.isConnected()) {
            spdlog::info("Нет подключения");
            return MENU_EXITS::DEFAULT;
        }

        std::string msisdn, msg;
        std::cout << "Введите номер получателя\n";
        std::cin>> msisdn;
        std::cout<<"Введите сообщение\n";
        std::cin>> msg;
        spdlog::info("Отправка сообщения");
        UEex.sendSMS(msisdn, msg);
        return MENU_EXITS::DEFAULT;
    }
};