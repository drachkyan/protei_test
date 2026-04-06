#include "../include/MME.h"
#include "../../ENODE/include/ENodeB.h"

MME::MME(const std::string& path)
        : worker(std::make_unique<MMEHandler>(path, xlr, ENodes)), xlr("xlr.db") {}


void MME::add_ENode(int id, ENodeB* enode) {
    std::lock_guard lock(enodeMtx);
    ENodes[id] = enode;
}

void MME::run() {
    auto worker_func = [this]() {
        while (!stop) {
            MMETask task;
            {
                std::unique_lock lock(taskMtx);
                taskCond.wait(lock, [this] { return !tasks.empty() || stop; });
                if (stop && tasks.empty()) return;
                task = std::move(tasks.front());
                tasks.pop();
            }
            json result = worker->handle(task.data);
            task.promise.set_value(result);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < MAX_THREADS; i++) {
        threads.emplace_back(worker_func);
    }
    for (auto& t : threads) t.join();
}


void MME::shutdown() {
    for (auto enode : ENodes) {
        enode.second->shutdown();
    }

    {
        std::lock_guard lock(taskMtx);
        stop = true;
    }
    taskCond.notify_all();
}

void MME::push(MMETask task) {
    {
        std::lock_guard lock(taskMtx);
        tasks.push(std::move(task));
    }
    taskCond.notify_one();
}
