#pragma once
#include "../../Menu/MenuItem.h"

class QuitItem : public AbstractMenuItem {
public:
    QuitItem (std::string name_, std::string description_):
        AbstractMenuItem(std::move(name_), std::move(description_))  {};

    MENU_EXITS action() override {
        spdlog::info("Выход из программы");
        return MENU_EXITS::QUIT;
    }
};