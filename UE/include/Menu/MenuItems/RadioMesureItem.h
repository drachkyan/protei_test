#pragma once
#include "../../Menu/MenuItem.h"
#include "../../Network/NetworkClient.h"
#include "../../../../model/Context/UEContext.h"

class RadioMeasureItem : public AbstractMenuItem {
    UEExchange& UEex;
    UEContext& ueCtx;
public:

    RadioMeasureItem (std::string name_, std::string description_, UEExchange& UEex_, UEContext& ueCtx_):
        AbstractMenuItem(std::move(name_), std::move(description_)), UEex(UEex_), ueCtx(ueCtx_)  {};

    json sendPos() {
        auto res = UEex.radioMeasure();
        return res;
    }

    MENU_EXITS action() override {
        if (!UEex.isConnected()) {
            spdlog::info("Не подключены к серверу");
            return MENU_EXITS::DEFAULT;
        }

        spdlog::info("Отправка местоположения");
        auto res = sendPos();
        spdlog::info("Результат: {}", res.dump());
        return MENU_EXITS::DEFAULT;
    }
};
