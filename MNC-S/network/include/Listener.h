#pragma once

#include <liburing.h>

#include <memory>
#include <list>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

enum class OpType {
    ACCEPT,
    RECV,
    SEND
};

struct OpContext {
    OpType type;
    int fd;
    std::unique_ptr<char[]> buf;
    size_t buf_size;
    bool isLastMessage;
    OpContext(OpType type_, int fd_, size_t size, bool flag = false)
        : type(type_), fd(fd_), buf(std::make_unique<char[]>(size)), buf_size(size), isLastMessage(flag) {}
};

using Handler = std::function<void(OpContext*, int)>;

class Listener {

protected:
    io_uring ring{};
    void addAccept();
    void addRecv(int client_fd);
    void addSend(int client_fd, const char* data, size_t len, bool isLast = false);

    virtual void onAccept(OpContext* ctx, int res) = 0;
    virtual void onRecv(OpContext* ctx, int res) = 0;
    virtual void onSend(const OpContext* ctx, int res) = 0;

private:
    const int PORT;
    const int QUEUE_DEPTH;
    const size_t BUF_SIZE;
    int server_fd;

    std::unordered_map<OpType, Handler> handlersMap;
    void initHandlers();
    std::list<std::unique_ptr<OpContext>> pendingContexts;
    bool init();



public:
    Listener(int PORT_);

    void run();

    virtual ~Listener() = default;

};
