#pragma once

#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

template<typename T>
std::optional<T> validate(const json& j) noexcept{
    try {
        return j.get<T>();
    } catch (...) {
        return std::nullopt;
    }
}