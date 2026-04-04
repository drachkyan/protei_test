#pragma once

#include <memory>
#include <string>

struct IpAddress {
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint8_t b4;

    IpAddress(const uint8_t b1, const uint8_t b2,
        const uint8_t b3, const uint8_t b4):
        b1(b1), b2(b2), b3(b3), b4(b4) {}

    [[nodiscard]] std::string toString() const {
        return std::to_string(b1) + "." +
               std::to_string(b2) + "." +
               std::to_string(b3) + "." +
               std::to_string(b4);
    }

    IpAddress& operator=(const IpAddress& other) {
        if (this != &other) {
            b1 = other.b1;
            b2 = other.b2;
            b3 = other.b3;
            b4 = other.b4;
        }
        return *this;
    }
};

class NetworkAddress {
    IpAddress addr{0,0,0,0};
    uint16_t port{};
    bool correctFlag = true;

    bool parseIpAddress(const char* str);
    uint16_t parsePort(const char* str);
public:
    [[nodiscard]] bool isCorrect() const { return correctFlag; }
    friend std::ostream& operator<<(std::ostream& os, const NetworkAddress& na);
    NetworkAddress(const std::string& port_, const std::string& addr_);

    [[nodiscard]] uint16_t getPort() const { return port; }
    [[nodiscard]] std::string getIpAddress() const { return addr.toString(); }

    NetworkAddress& operator=(const NetworkAddress& other) {
        if (this != &other) {
            addr = other.addr;
            port = other.port;
            correctFlag = other.correctFlag;
        }
        return *this;
    }
};
