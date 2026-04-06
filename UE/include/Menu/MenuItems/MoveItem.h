#pragma once

#include "../MenuItem.h"
#include <iostream>

class MoveItem : public AbstractMenuItem {
    Context& UEctx;
public:
    MoveItem (std::string name_, std::string description_, Context& ctx):
        AbstractMenuItem(std::move(name_), std::move(description_)), UEctx(ctx)  {};

    MENU_EXITS action() override {
        double x = 0;
        std::cout<<"Введите координату\n";
        std::cin>>x;
        UEctx.move(x);
        return MENU_EXITS::DEFAULT;
    }
};