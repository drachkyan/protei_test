#pragma once

#include "../../Menu/MenuItem.h"
#include "../../Network/Exchange.h"

class ActivateItem : public AbstractMenuItem {
    Exchange& UEex;

public:
    ActivateItem (std::string name_, std::string description_, Exchange& UEex_):
            AbstractMenuItem(std::move(name_), std::move(description_)), UEex(UEex_)  {};
    MENU_EXITS action() override {
        if (UEex.isConnected()) {
            spdlog::info("Уже подключены");
            return MENU_EXITS::DEFAULT;
        }
        UEex.connect();
        return MENU_EXITS::DEFAULT;
    }
};
