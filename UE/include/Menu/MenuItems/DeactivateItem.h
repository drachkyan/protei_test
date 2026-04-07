#pragma once

#include "../../Menu/MenuItem.h"
#include "../../Network/Exchange.h"

class DeactivateItem : public AbstractMenuItem {
    Exchange& UEex;

public:
    DeactivateItem (std::string name_, std::string description_, Exchange& UEex_):
            AbstractMenuItem(std::move(name_), std::move(description_)), UEex(UEex_)  {};
    MENU_EXITS action() override {
        if (!UEex.isConnected()) {
            spdlog::info("Не подключены");
            return MENU_EXITS::DEFAULT;
        }
        UEex.shutdown();
        return MENU_EXITS::DEFAULT;
    }
};
