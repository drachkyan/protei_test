#pragma once

#include "../Settings/AppSettings.h"
#include <nlohmann/json.hpp>
#include "../../include/utils/utils.h"

using json = nlohmann::json;


void toLowerCase(std::string &c);

size_t hash(const char* str);

