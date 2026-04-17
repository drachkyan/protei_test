#include "../include/ENodeB.h"
#include <spdlog/spdlog.h>


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
    spdlog::info("[ENODE{}] Количество подключений сейчас = {}",config.id, TMSItoSlots.size()+1);
    return TMSItoSlots.size() <= MAX_CONNECTIONS - 1;  // сначала сравниаем поэтому минус один
}

bool ENodeB::reserveSlot(const std::string& tmsi) {
    std::lock_guard lock(slotMtx);
    spdlog::info("[ENODE{}] Резервируем слот для {}", config.id, tmsi);

    if (!hasFreeSlot()) {
        spdlog::info("[ENODE{}] нет свободных слотов", config.id);
        return false;
    }

    TMSItoSlots.emplace(tmsi, Slot{});
    return true;
}

void ENodeB::releaseSlot(const std::string& tmsi) {
    std::lock_guard lock(slotMtx);
    TMSItoSlots.erase(tmsi);
}

void ENodeB::handover(std::string tmsi, Slot slot) {
    std::lock_guard lock(slotMtx);

    TMSItoSlots[tmsi] = std::move(slot);

    spdlog::info("[ENODE {}] Принят handover для TMSI {}. Буфер перенесен.", config.id, tmsi);
}

std::optional<Slot> ENodeB::detachSlot(const std::string &tmsi) {
    std::lock_guard lock(slotMtx);
    auto it = TMSItoSlots.find(tmsi);
    if (it != TMSItoSlots.end()) {
        Slot movedSlot = std::move(it->second);
        TMSItoSlots.erase(it);

        return movedSlot;
    }
    return std::nullopt;
}

void ENodeB::addSMStoSlot(const std::string &tmsi_s, const std::string &msisdn_d, SMSMessage &msg) {
    std::lock_guard lock(slotMtx);
    spdlog::info("[ENODE{}] добавлено сообщение для {}", config.id, msisdn_d);
    auto& queue = TMSItoSlots[tmsi_s].outbox[msisdn_d];
    queue.push(msg);
}

bool ENodeB::deleteSMSfromSlot(const std::string &tmsi_s, const std::string &msisdn_d) {
    std::lock_guard lock(slotMtx);
    auto slotIt = TMSItoSlots.find(tmsi_s);
    if (slotIt == TMSItoSlots.end()) {
        spdlog::warn("[ENODE{}] Удаление смс: TMSI {} не найден",config.id, tmsi_s);
        return false;
    }

    auto& outbox = slotIt->second.outbox;
    auto queueIt = outbox.find(msisdn_d);
    if (queueIt == outbox.end() || queueIt->second.empty()) {
        spdlog::warn("[ENODE{}] Удаление смс: TMSI {} не найден",config.id, tmsi_s);
        return false;
    }

    queueIt->second.pop();
    return true;
}

void ENodeB::receiveSMS(SMSMessage msg) {
    {
        std::lock_guard lock(slotMtx);
        auto& queue = TMSItoSlots[msg.tmsi_dst].inbox;
        queue.push(msg);
    }
    // отправляем клиенту
    json req {
        {"type", "SS"},
        {"TMSI_D", msg.tmsi_dst}
    };

    std::promise<json> a;
    this->pushInternalTask(Task{req, std::move(a)});
}


std::optional<SMSMessage> ENodeB::getSMStoSend(const std::string &tmsi_s, const std::string &msisdn_d) {
    auto slotIt = TMSItoSlots.find(tmsi_s);
    if (slotIt == TMSItoSlots.end()) {
        spdlog::info("[ENODE{}] Слот для TMSI {} не найден", config.id, tmsi_s);
        return std::nullopt;
    }

    auto& outboxMap = slotIt->second.outbox;
    auto outboxIt = outboxMap.find(msisdn_d);

    if (outboxIt == outboxMap.end() || outboxIt->second.empty()) {
        spdlog::info("[ENODE{}] Очередь сообщений для {} пуста", config.id, msisdn_d);
        return std::nullopt;
    }

    auto& queue = outboxIt->second;
    SMSMessage msg = std::move(queue.front());
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


