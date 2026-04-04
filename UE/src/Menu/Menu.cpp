#include "../../include/Menu/Menu.h"
#include <iostream>
#include <string>

#include "../../include/Menu/MenuItems/ActivateItem.h"
#include "spdlog/spdlog.h"
#include "../../include/utils/utils.h"
#include "../../include/Network/NetworkClient.h"
#include "../../include/Menu/MenuItems/QuitItem.h"
#include "../../include/Menu/MenuItems/RadioMesureItem.h"

void Menu::initMenuItems() {
    menuItems.insert({"quit",
        std::make_unique<QuitItem>("quit", "Выход из программы")});

    menuItems.insert({"exit",
        std::make_unique<QuitItem>("", "")});

    menuItems.insert({"send",
        std::make_unique<RadioMeasureItem>("send", "Отправить местоположение TEST", UEex, UEctx)});

    menuItems.insert({"activate",
        std::make_unique<ActivateItem>("activate", "Активировать подключение", UEex)});

}

Menu::Menu(AppSettings &app_): app(app_), UEex(app_), UEctx(app.getContext()) {
    initMenuItems();
}

void Menu::run() const {

    std::cout << *this;
    MENU_EXITS EXIT_CODE = MENU_EXITS::DEFAULT;
    std::string command;
    while (EXIT_CODE == MENU_EXITS::DEFAULT ) {

        if (!(std::cin >> command)) {
            spdlog::info("Произошла ошибка");
            break;
        }

        toLowerCase(command);
        auto it = menuItems.find(command);
        if (it == menuItems.end()) {
            spdlog::info("Команда не найдена");
            continue;
        }

        EXIT_CODE = it->second->action();
    }
    fclose(stdin);
}
