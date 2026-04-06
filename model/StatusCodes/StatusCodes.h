#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace StatusCode {
    constexpr int SUCCESS = 200;
    constexpr int BAD_REQUEST = 400;
    constexpr int NOT_FOUND = 404;
    constexpr int SERVER_ERROR = 500;

    const json SUCCESS_JSON = {{"status", SUCCESS}};
    const json NOT_FOUND_JSON = {{"status", NOT_FOUND}};
    const json SERVER_ERROR_JSON = {{"status", SERVER_ERROR}};
    const json BAD_REQUEST_JSON = {{"status", BAD_REQUEST}};
}

enum class MessageStatus {
    PENDING,
    SENT,
    DELIVERED,
    FAILED
};

inline MessageStatus stringToMessageStatus(const std::string& status_str) {
    static const std::unordered_map<std::string, MessageStatus> statusMap {
            {"pending",   MessageStatus::PENDING},
            {"sent",      MessageStatus::SENT},
            {"delivered", MessageStatus::DELIVERED},
            {"failed",    MessageStatus::FAILED},
            {"read",      MessageStatus::DELIVERED}
    };

    auto it = statusMap.find(status_str);
    return (it != statusMap.end()) ? it->second : MessageStatus::FAILED;
}


inline json getJsonMessageStatus(MessageStatus status) {
    switch (status) {
        case MessageStatus::PENDING:   return {{"message_status", "pending"}};
        case MessageStatus::SENT:      return {{"message_status", "sent"}};
        case MessageStatus::DELIVERED: return {{"message_status", "delivered"}};
        default:                       return {{"message_status", "failed"}};
    }
}