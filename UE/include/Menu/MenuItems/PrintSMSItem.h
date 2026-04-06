#pragma once
#include <iostream>

#include "../../Menu/MenuItem.h"

class PrintSMSItem : public AbstractMenuItem {
    Context& UEctx;

    void printChat(std::string& msisdn) {
        auto chat = UEctx.getChat(msisdn);
        if (!chat) {
            spdlog::info("Сообщений с {} еще нет", msisdn);
            return;
        }

        using SMSPair = std::pair<SMSRecord, bool>;

        std::vector<SMSPair> all;
        for (auto& msg : chat->incoming) {
            all.push_back({msg, true});
        }

        for (auto& msg : chat->outgoing) {
            all.push_back({msg.second, false});
        }

        std::sort(all.begin(), all.end(), [](const SMSPair& a, const SMSPair& b) {
            return a.first.time < b.first.time;
        });

        for (auto& [msg, isIncoming] : all) {
            if (isIncoming) {
                std::cout << "[" << msg.msisdn << "]: " << msg.text << "\n";
            } else {
                std::cout << "[Я]: " << msg.text << "\n";
            }
        }
    }
public:
    PrintSMSItem (std::string name_, std::string description_, Context& UEctx_):
        AbstractMenuItem(std::move(name_), std::move(description_)), UEctx(UEctx_)  {};

    MENU_EXITS action() override {
        std::string msisdn{};
        std::cout<<"Введите номер чата для просмотра чата\n";
        std::cin>>msisdn;
        printChat(msisdn);
        return MENU_EXITS::DEFAULT;
    }
};