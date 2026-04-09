#include "../include/Listener.h"

#include <netinet/in.h>
#include <spdlog/spdlog.h>

using json = nlohmann::json;


void Listener::initHandlers() {
    handlersMap[OpType::ACCEPT] = [this](OpContext* ctx, int res) { onAccept(ctx, res); };
    handlersMap[OpType::RECV]   = [this](OpContext* ctx, int res) { onRecv(ctx, res); };
    handlersMap[OpType::SEND]   = [this](OpContext* ctx, int res) { onSend(ctx, res); };
}

void Listener::addAccept() {
    auto ctx = std::make_unique<OpContext>(OpType::ACCEPT, server_fd, BUF_SIZE);
    auto* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_accept(sqe, server_fd,
        nullptr, nullptr, 0);
    io_uring_sqe_set_data(sqe, ctx.get());
    pendingContexts.push_back(std::move(ctx));
}

void Listener::addRecv(int client_fd) {
    auto ctx = std::make_unique<OpContext>(OpType::RECV, client_fd, BUF_SIZE);
    auto* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_recv(sqe, client_fd, ctx->buf.get(), ctx->buf_size, 0);
    io_uring_sqe_set_data(sqe, ctx.get());
    pendingContexts.push_back(std::move(ctx));
}

void Listener::addSend(int client_fd, const char *data, size_t len, bool isLast) {
    auto ctx = std::make_unique<OpContext>(OpType::SEND, client_fd, len, isLast);
    std::copy_n(data, len, ctx->buf.get());
    auto* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_send(sqe, client_fd, ctx->buf.get(), len, 0);
    io_uring_sqe_set_data(sqe, ctx.get());
    pendingContexts.push_back(std::move(ctx));
}

Listener::Listener(int PORT_):
    PORT(PORT_), QUEUE_DEPTH(32), BUF_SIZE(1024) {
    initHandlers();
}

void Listener::run() {
    if (!init()) {
        spdlog::info("Сервер не запущен");
        return;
    }

    addAccept();
    io_uring_submit(&ring);

    while (true) {
        io_uring_cqe* cqe = nullptr;

        if (io_uring_wait_cqe(&ring, &cqe) < 0) {
            spdlog::info( "ошибка wait_cqe\n");
            break;
        }

        auto* ctx = static_cast<OpContext*>(io_uring_cqe_get_data(cqe));

        auto ctx_ptr = std::find_if(pendingContexts.begin(), pendingContexts.end(),
            [ctx](const auto& p) { return p.get() == ctx; });

        int res = cqe->res;
        io_uring_cqe_seen(&ring, cqe);

        try {
            handlersMap[ctx->type](ctx, res);
        }catch (std::exception& e) {
            spdlog::error(e.what());
        }
        pendingContexts.erase(ctx_ptr);

    }
}

bool Listener::init() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        spdlog::info("ошибка создания сокета");
    }

    const int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {.s_addr = INADDR_ANY}
    };

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        spdlog::info("ошибка - скорее всего порт занят");
        return false;
    }
    if (listen(server_fd, 10) < 0) {
        spdlog::info("ошибка запуска сервера");
        return false;
    }

    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        spdlog::info("ошибка создания io_uring");
        return false;
    }
    spdlog::info("[сервер]: запущен на порту {}\n", PORT);
    return true;
}

