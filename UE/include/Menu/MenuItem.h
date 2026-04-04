#pragma once

#include <string>
#include <queue>
#include "../utils/utils.h"

enum class MENU_EXITS {
    QUIT, DEFAULT
};

class AbstractMenuItem {
public:
    virtual ~AbstractMenuItem() = default;

    virtual MENU_EXITS action() = 0;
    std::string name;
    std::string description;
    AbstractMenuItem (std::string name_, std::string description_):
        name(std::move(name_)), description(std::move(description_)) {};

    friend std::ostream& operator<<(std::ostream& os, const AbstractMenuItem& item) {
        if (item.description.empty()) { return os; }
        os << item.name << " - " << item.description << std::endl;
        return os;
    }
};



