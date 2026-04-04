#include "../include/Gateway.h"

#include <netinet/in.h>

json Gateway::handleManyNodes(json &req) const {

    std::vector<std::future<json>> futures;
    json res{};
    for (auto& enb : ENodes) {
        std::promise<json> resPromise;
        futures.push_back(resPromise.get_future());
        enb.second->push(Task{req, std::move(resPromise)});
    }

    json results = json::array();
    for (auto& f : futures) {
        auto res = f.get();
        if (!res.empty()) {
            results.push_back(res);
        }
    }

    res["ENodes"] = results;
    return res;
}

json Gateway::handleNode(json &req, int id) const {
    json res{};
    if (ENodes.find(id) == ENodes.end()) {
        spdlog::info("Не найдена базовая станция");
        res["status"] = "ENode is not found";
        return res;
    }

    std::promise<json> resPromise;
    auto resFuture = resPromise.get_future();
    ENodes[id]->push(Task{req, std::move(resPromise)});
    res = resFuture.get();
    return res;

}

void Gateway::onAccept(OpContext *ctx, int res) {
    if (res < 0) {
        spdlog::info("ошибка\n");
        return;
    }
    int client_fd = res;
    clients[client_fd] = ClientState{};
    spdlog::info("новый клиент fd={}\n" , client_fd);

    addAccept();
    addRecv(client_fd);
    io_uring_submit(&ring);
}

void Gateway::onRecv(OpContext *ctx, int res) {
    if (res <= 0) {
        if (clients.find(ctx->fd) == clients.end()) {
            return;
        }
        spdlog::info( "клиент fd={} отключился\n", ctx->fd);
        clients.erase(ctx->fd);
        close(ctx->fd);
        return;
    }


    auto& state = clients[ctx->fd];

    state.buffer.insert(state.buffer.end(), ctx->buf.get(), ctx->buf.get() + res); //считал

    if (state.reading_len && state.buffer.size()>= sizeof(uint32_t)) {
        uint32_t len;
        std::memcpy(&len, state.buffer.data(), sizeof(uint32_t));
        state.expected_len = ntohl(len);
        state.buffer.erase(state.buffer.begin(), state.buffer.begin() + sizeof(uint32_t));
        state.reading_len = false;
    } //пакет с длинной

    if (!state.reading_len && state.buffer.size() >= state.expected_len) {
        std::string json_str(state.buffer.begin(), state.buffer.begin() + state.expected_len);
        state.buffer.erase(state.buffer.begin(), state.buffer.begin() + state.expected_len);
        state.reading_len = true;


        auto response = packageProcess(json_str.c_str());

        std::string data = response.dump();
        uint32_t len = htonl(data.size());

        addSend(ctx->fd, reinterpret_cast<const char*>(&len), sizeof(len));
        addSend(ctx->fd, data.c_str(), data.size(), true);
        io_uring_submit(&ring);
        return;
    }

    addRecv(ctx->fd);
    io_uring_submit(&ring);
}

void Gateway::onSend(const OpContext *ctx, int res) {
    if (res < 0) {
        spdlog::info("ошибка отправки: {}\n", std::strerror(-res));
        return;
    }
    if (ctx->isLastMessage) {
        addRecv(ctx->fd);
        io_uring_submit(&ring);
    }
}
