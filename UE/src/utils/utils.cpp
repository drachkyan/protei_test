#include "../../include/utils/utils.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;


void toLowerCase(std::string &c) {
    size_t i = 0;
    for (size_t j = 0; j < c.size(); ++j) {
        c[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(c[i])));
        i++;
    }
}


size_t hash(const char* str) {
    size_t hash = 0;
    while (*str) {
        hash = hash * 31 + *str;
        ++str;
    }
    return hash;
}
