#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <utility>

#include "../../../model/Context/UEContext.h"
#include "../Menu/MenuItem.h"

#include "../Settings/AppSettings.h"
#include "../Network/NetworkClient.h"
#include "../Network/UEExchange.h"

using menu_func_type = std::function<u_int16_t()>;
using creator_func_type = std::function<void*()>;


class Menu {
    AppSettings& app; // нужно для всего остального, не переносить
    UEContext UEctx;
    UEExchange UEex;
    std::string alias;

    std::unordered_map<std::string, std::unique_ptr<AbstractMenuItem>> menuItems;
    void initMenuItems();
public:

    Menu(AppSettings& app_);

    friend std::ostream& operator<<(std::ostream& os, const Menu& menu) {
        os << "======== МЕНЮ =========" << std::endl;
        for (const auto& item: menu.menuItems) {
            os << *item.second;
        }
        return os;
    }
    void run() const;
};

