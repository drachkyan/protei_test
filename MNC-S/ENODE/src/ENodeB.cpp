#include "../include/ENodeB.h"

#include "spdlog/spdlog.h"


ENodeB::ENodeB(int id_, double x_, double power_, double radius_, MME& mme_):
        config(id_, x_, power_, radius_), mme(mme_),
        worker(std::make_unique<EnodeHandler>(config,mme)) {

    mme.add_ENode(id_, this);
    slots.reserve(MAX_CONNECTIONS);
}


void ENodeB::run() {
    while (!stop) {
        Task task;

        {
            std::unique_lock lock(taskMtx);

            taskCond.wait(lock, [this] {
                return !tasks.empty();
            });

            if (stop && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        }

        json result = worker->handle(task.data);

        task.promise.set_value(result);
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

bool ENodeB::hasFreeSlot() const {
    return slots.size() <= MAX_CONNECTIONS;
}

bool ENodeB::reserveSlot(const std::string& tmsi) {
    std::lock_guard lock(slotMtx);
    if (!hasFreeSlot()) {
        spdlog::info("[ENODE] нет свободных слотов");
        return false;
    }

    slots[tmsi] = Slot{};
    return true;
}

void ENodeB::releaseSlot(const std::string& tmsi) {
    std::lock_guard lock(slotMtx);
    slots.erase(tmsi);
}