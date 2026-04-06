#include "../include/ENodeB.h"

#include "spdlog/spdlog.h"


ENodeB::ENodeB(int id_, double x_, double power_, double radius_, MME& mme_, std::unordered_map<int, ENodeB*>& ENodes_, Sender& sender_):
        config(id_, x_, power_, radius_), mme(mme_),
        worker(std::make_unique<EnodeHandler>(config,mme, ENodes_, sender_) ) {

    mme.add_ENode(id_, this);
    TMSItoSlots.reserve(MAX_CONNECTIONS);
}


void ENodeB::run() {

    while (!stop) {
        std::unique_lock lock(taskMtx);
        taskCond.wait(lock, [this] {
            return !tasks.empty() || !internalTasks.empty() || stop;
        });

        if (stop && tasks.empty() && internalTasks.empty()) {
            return;
        }

        if (!internalTasks.empty()) {
            auto task = std::move(internalTasks.front());
            internalTasks.pop();
            lock.unlock();

            json result = worker->handleInternalTask(task.data);
            task.promise.set_value(result);

        } else {
            auto task = std::move(tasks.front());
            tasks.pop();
            lock.unlock();

            json result = worker->handle(task.data);
            task.promise.set_value(result);
        }
    }
}

void ENodeB::shutdown() {
    {
        std::lock_guard lock(taskMtx);
        stop = true;
    }
    taskCond.notify_all();
}

void ENodeB::push(Task msg) {
    {
        std::lock_guard lock(taskMtx);
        tasks.push(std::move(msg));
    }
    taskCond.notify_one();
}

void ENodeB::pushInternalTask(Task msg) {
    {
        std::lock_guard lock(taskMtx);
        internalTasks.push(std::move(msg));
    }
    taskCond.notify_one();
}

bool ENodeB::hasFreeSlot() const {
    return TMSItoSlots.size() <= MAX_CONNECTIONS;
}

bool ENodeB::reserveSlot(const std::string& tmsi) {
    std::lock_guard lock(slotMtx);
    if (!hasFreeSlot()) {
        spdlog::info("[ENODE] нет свободных слотов");
        return false;
    }

    TMSItoSlots[tmsi] = Slot{};
    return true;
}

void ENodeB::releaseSlot(const std::string& tmsi) {
    std::lock_guard lock(slotMtx);
    TMSItoSlots.erase(tmsi);
}

void ENodeB::addSMStoSlot(const std::string &tmsi_s, const std::string &msisdn_d, SMSMessage &msg) {
    std::lock_guard lock(slotMtx);
    spdlog::info("[ENODE] добавлено сообщение для {}", msisdn_d);
    auto& queue = TMSItoSlots[tmsi_s].outbox[msisdn_d];
    queue.push(msg);
}

void ENodeB::receiveSMS(SMSMessage msg) {
    {
        std::lock_guard lock(slotMtx);
        auto& queue = TMSItoSlots[msg.tmsi_dst].inbox;
        queue.push(msg);
    }
    spdlog::info("tmsi: {}", msg.tmsi_dst);
    // отправляем клиенту
    json req {
        {"type", "SS"},
        {"TMSI_D", msg.tmsi_dst}
    };

    std::promise<json> a;
    this->pushInternalTask(Task{req, std::move(a)});
}


SMSMessage ENodeB::getSMStoSend(const std::string &tmsi_s, const std::string &msisdn_d) {
    std::lock_guard lock(slotMtx);
    spdlog::info("[ENODE] ищем сообщение для {}", msisdn_d);
    auto& queue = TMSItoSlots[tmsi_s].outbox[msisdn_d];
    SMSMessage msg = queue.front();
    spdlog::info("Нашли: {}", msg.text);
    queue.pop();
    return msg;
}

SMSMessage ENodeB::getSMSbyTMSI(const std::string &tmsi) {
    std::lock_guard lock(slotMtx);
    auto& queue = TMSItoSlots[tmsi].inbox;
    SMSMessage msg = queue.front();
    queue.pop();
    return msg;
}


