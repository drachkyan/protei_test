#include "../../include/Menu/Menu.h"
#include <iostream>
#include <string>

#include "../../include/Menu/MenuItems/ActivateItem.h"
#include "../../include/Menu/MenuItems/MoveItem.h"
#include "spdlog/spdlog.h"
#include "../../include/utils/utils.h"
#include "../../include/Menu/MenuItems/QuitItem.h"
#include "../../include/Menu/MenuItems/SendSMSItem.h"
#include "../../include/Menu/MenuItems/PrintSMSItem.h"
#include "../../include/Menu/MenuItems/DeactivateItem.h"

void Menu::initMenuItems() {
    menuItems.insert({"quit",
        std::make_unique<QuitItem>("quit", "Выход из программы")});

    menuItems.insert({"exit",
        std::make_unique<QuitItem>("", "")});

    menuItems.insert({"activate",
        std::make_unique<ActivateItem>("activate", "Активировать подключение", UEex)});

    menuItems.insert({"send_message",
        std::make_unique<SendSMSItem>("send_message", "Отправить сообщение", UEex, app.getContext())});

    menuItems.insert({"open_chat",
        std::make_unique<PrintSMSItem>("open_chat", "Открыть чат по номеру телефона", app.getContext())});

    menuItems.insert({"move",
        std::make_unique<MoveItem>("move", "Переместится на другую координату", app.getContext())});

    menuItems.insert({"deactivate",
        std::make_unique<DeactivateItem>("deactivate", "Деактивировать подключение", UEex)});

}

Menu::Menu(AppSettings &app_): app(app_), UEex(app_) {
    initMenuItems();
}

void Menu::run() {

    std::cout << *this;
    MENU_EXITS EXIT_CODE = MENU_EXITS::DEFAULT;
    std::string command;
    while (EXIT_CODE == MENU_EXITS::DEFAULT ) {
        std::cout << std::flush;
        if (!std::getline(std::cin, command)) {
            if (std::cin.eof()) {
                spdlog::info("Достигнут конец потока (EOF)");
            } else {
                spdlog::error("Ошибка чтения ввода");
            }
            break;
        }
        command.erase(0, command.find_first_not_of(" \t\r\n"));
        command.erase(command.find_last_not_of(" \t\r\n") + 1);

        if (command.empty()) {
            continue;
        }

        toLowerCase(command);
        auto it = menuItems.find(command);
        if (it == menuItems.end()) {
            spdlog::info("Команда не найдена");
            continue;
        }

        EXIT_CODE = it->second->action();
    }
    UEex.shutdown();
    fclose(stdin);
}
