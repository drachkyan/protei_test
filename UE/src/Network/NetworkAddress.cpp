#include "../../include/Network/NetworkAddress.h"

#include <string>
#include "spdlog/spdlog.h"
#include <iostream>

bool NetworkAddress::parseIpAddress(const char* str) {
    uint8_t bytes[4];
    char* end = nullptr;
    size_t counter = 0;



    for (int64_t i_ = std::strtol(str, &end, 10);
        str != end;
        i_ = std::strtol(str, &end, 10)) {
            if (i_ < 0 || i_ > 255) {
                break;
            }
            bytes[counter] = i_;
            str = end;
            if (*str == '.') {
                ++str;
            }
            counter++;
            if (counter == 4) {
                break;
            }
        }
    if (counter != 4 || *end != '\0') {
        spdlog::info("Ошибка во время парсинга айпи адреса");
        return false;
    }
    addr = {bytes[0], bytes[1], bytes[2], bytes[3]};

    return true;
}


uint16_t NetworkAddress::parsePort(const char *str) {
    char* end = nullptr;

    const int64_t tempPort = strtol(str, &end, 10);

    if (*end != '\0') {
        spdlog::info("Ошибка при парсинге порта");
        return 0;
    }
    if (constexpr int64_t MAX_PORT = 65535;
        tempPort > MAX_PORT || tempPort <= 0) {
        spdlog::info("Порт должен быть в диапазоне 1 - 65535");
        return 0;
        }
    return tempPort;
}


NetworkAddress::NetworkAddress(const std::string& port_, const std::string& addr_) {
    port = parsePort(port_.c_str());
    bool addrOk = parseIpAddress(addr_.c_str());
    if (port == 0 || !addrOk) {
        correctFlag = false;
    }
}


std::ostream& operator<<(std::ostream& os, const NetworkAddress& na) {
    if (!na.isCorrect() || na.port == 0) {
        os << "invalid address";
        return os;
    }
    os << static_cast<int>(na.addr.b1) << "."
       << static_cast<int>(na.addr.b2) << "."
       << static_cast<int>(na.addr.b3) << "."
       << static_cast<int>(na.addr.b4) << ":"
       << na.port;
    return os;
}