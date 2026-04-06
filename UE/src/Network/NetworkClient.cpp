#include "../../include/Network/NetworkClient.h"

#include <future>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include "spdlog/spdlog.h"
#include "../../include/utils/utils.h"


int NetworkClient::createConnection() {
    fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{
        .sin_family = AF_INET,
        .sin_port   = htons(address.getPort()),
    };
    inet_pton(AF_INET, address.getIpAddress().c_str(), &addr.sin_addr);

    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        return 1;
    }
    spdlog::info("Сеть обнаружена");
    connectionFlag=true;
    return 0;
}

void NetworkClient::close() {
    if (fd != -1) {
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
        fd = -1;
    }
}

void NetworkClient::sendJSON(const json &j) const {
    if (fd == -1) {
        spdlog::info("Нет соединения");
        return;
    }
    std::string data = j.dump();
    uint32_t len = htonl(data.size());
    send(fd, &len, sizeof(len), 0);
    send(fd, data.c_str(), data.size(), 0);
}

json NetworkClient::recvJSON() {
    if (fd == -1) {
        spdlog::info("Нет соединения");
        return json{};
    }
    uint32_t len;
    int n = recv(fd, &len, sizeof(len), 0);

    if (n <= 0) {
        connectionFlag = false;
        fd = -1;
        return json{};
    }

    len = ntohl(len);

    if (len == 0 || len > 1024 * 1024) {
        spdlog::info("Некорректная длина пакета: {}", len);
        return json{};
    }

    std::string buf(len, '\0');
    n = recv(fd, buf.data(), len, 0);
    if (n <= 0) {
        spdlog::info("Сервер отключился при чтении данных");
        connectionFlag = false;
        fd = -1;
        return json{};
    }

    return json::parse(buf);
}



NetworkClient::NetworkClient(const NetworkAddress& address_)
                                : address(address_)
{
    if (!address.isCorrect()) {
        spdlog::info("Айпи адрес или порт введены неверно");
        return;
    }
}

NetworkClient::~NetworkClient() {
    connectionFlag = false;

    if (fd != -1) {
        ::close(fd);
    }
}
