#include "../../include/Network/Exchange.h"
#include "../../../model/StatusCodes/StatusCodes.h"

bool Exchange::isHandoverNeeded(double cur, double best) {
    const double THRESHOLD = 0.1;
    return (best - cur) > THRESHOLD;
}

json Exchange::handleAttachRequest() {
    json req = {
        {"type", "A"},
        {"IMSI", settings.getContext().getIMSI()},
        {"IMEI", settings.getContext().getIMEI()},
        {"MSISDN", settings.getContext().getMSISDN()},
        {"ENode", enodebId}
    };
    auto res = sendAndWait(req);
    return res;
}

json Exchange::handleAuthResponse() {
    json req = {
        {"type", "AR"},
        {"TMSI", settings.getContext().getTMSI()},
        {"IMEI", settings.getContext().getIMEI()},
        {"ENode", enodebId}
    };

    auto res = sendAndWait(req);
    return res;
}


void Exchange::run() {

    while (IN_ACTIVE) {
        auto msg = api.recvJSON();
        if (msg.empty()) {
            IN_ACTIVE = false;
            break;
        }

        if (!msg.contains("id") || msg["id"].get<int>() == -1) {
            worker.handle(msg);
            continue;
        }

        int id = msg["id"].get<int>();
        std::lock_guard lock(pendingMtx);
        auto it = pending.find(id);
        if (it != pending.end()) {
            it->second.set_value(msg);
            pending.erase(it);
        } else {
            spdlog::error("Неизвестный id: {}", id);
        }
    }
}

json Exchange::sendAndWait(json &req) {
    int id = ++requestId;
    req["id"] = id;
    std::promise<json> promise;
    auto future = promise.get_future();
    {
        std::lock_guard lock(pendingMtx);
        pending[id] = std::move(promise);
    }
    api.sendJSON(req);
    return future.get();
}

void Exchange::send(json &req) {
    req["id"] = -1;
    api.sendJSON(req);
}

json Exchange::radioMeasure() {
    json req{};
    req["type"] = "R";

    if (!settings.getContext().getTMSI().empty()) {
        req["TMSI"] = settings.getContext().getTMSI();
    }else {
        req["IMSI"] = settings.getContext().getIMSI();
    }

    req["pos"] = settings.getContext().getX();
    auto res = sendAndWait(req);
    return res;
}

void Exchange::handleStationLoss() {
    spdlog::info("Вышли за пределы базовой станции {} - переподключаемся", enodebId);
    onDisconnect();

    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        connect();
    }).detach();

}

void Exchange::checkAndPerformHandover(const json& enodes, double currentPower) {

    auto bestIt = std::ranges::max_element(enodes, [](const json& a, const json& b) {
        return a["power"].get<double>() < b["power"].get<double>();
    });

    if (bestIt == enodes.end()) {
        return;
    }
    int bestId = (*bestIt)["ENode"].get<int>();
    double bestPower = (*bestIt)["power"].get<double>();
    if (bestId != enodebId && isHandoverNeeded(currentPower, bestPower)) {

        if (handover(bestId)) {
            enodebId = bestId;
        }
    }
}

void Exchange::signalWorker() {
    double currentPower = -1;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while (IN_ACTIVE) {
        std::unique_lock lock(signalMtx);
        signalCv.wait_for(lock, std::chrono::seconds(2));

        auto res = radioMeasure();
        if (!res.contains("ENodes") || res["ENodes"].empty()) {
            spdlog::info("Нет сигнала - отключение");
            onDisconnect();
            return;
        }

        auto& enodes = res["ENodes"];
        auto currentIt = std::ranges::find_if(enodes, [this](const json& node) {
            return node["ENode"].get<int>() == enodebId;
        });

        if (currentIt == enodes.end()) {
            handleStationLoss();
            continue;
        }

        currentPower = (*currentIt)["power"].get<double>();
        checkAndPerformHandover(enodes, currentPower);


    }
}

void Exchange::onDisconnect() {
    if (!IN_ACTIVE) return;
    spdlog::info("Отключение от сервера");
    IN_ACTIVE = false;
    settings.getContext().clearTMSI();
    signalCv.notify_all();
    api.close();

    std::lock_guard lock(pendingMtx);
    for (auto& [id, promise] : pending) {
        promise.set_value(StatusCode::BAD_REQUEST_JSON);
    }
    pending.clear();
}

bool Exchange::handover(int ENode_) {
    json req{
        {"type", "H"},
        {"TMSI", settings.getContext().getTMSI()},
        {"ENode", enodebId},
        {"ENode_D", ENode_}
    };
    auto res = sendAndWait(req);
    if (!res.contains("status") || StatusCode::SUCCESS != res["status"].get<int>()) {
        return false;
    }
    return true;
}

void Exchange::shutdown() {
    onDisconnect();
}

bool Exchange::attach() {
    auto enodesPower = radioMeasure();

    if (!enodesPower.contains("ENodes") || enodesPower["ENodes"].empty()) {
        spdlog::error("Нет доступных базовых станций");
        return false;
    }

    auto& enodes = enodesPower["ENodes"];

    std::sort(enodes.begin(), enodes.end(), [](const json& a, const json& b)
                                  {
                                        return a["power"].get<double>() > b["power"].get<double>();
                                  });

    for (auto enode : enodes) {
        enodebId = enode["ENode"].get<int>();

        auto res = handleAttachRequest();

        if (res.contains("status") && StatusCode::SUCCESS == res["status"].get<int>()) {
            settings.getContext().setTMSI(res["TMSI"]);
            auto authConfirm = handleAuthResponse();

            if (authConfirm.contains("status") && StatusCode::SUCCESS == authConfirm["status"].get<int>()) {
                IN_ACTIVE = true;
                spdlog::info("Успешно прикрепились к ENode {}", enodebId);
                return true;
            }
            settings.getContext().clearTMSI();

        }
    }

    return false;

}

void Exchange::sendSMS(const std::string &msisdn, const std::string &msg) {
    SMSRecord sms = {msisdn, msg, MessageStatus::PENDING, std::chrono::system_clock::now()};
    auto id = settings.getContext().addSMS(msisdn, sms);

    json req {
        {"type", "M"},
        {"TMSI_S", settings.getContext().getTMSI()},
        {"MSISDN_D", msisdn},
        {"SMS", msg},
        {"SMS_ID", id},
        {"ENode", enodebId}
    };

    auto res = sendAndWait(req);

    if (!res.contains("status")) {
        spdlog::info("Ошибка отправки");
        settings.getContext().changeSMSStatus(msisdn, id, MessageStatus::FAILED);
        return;
    }
    if (StatusCode::SUCCESS == res["status"]) {
        spdlog::info("Сообщение отправлено");
        settings.getContext().changeSMSStatus(msisdn, id, MessageStatus::SENT);
        return;
    }
    settings.getContext().changeSMSStatus(msisdn, id, MessageStatus::FAILED);
    spdlog::info("Произошла ошибка при отправке");

}

void Exchange::sendSMSStatus(const std::string &msisdn_d, int id, MessageStatus status) {
    json req = getJsonMessageStatus(status);
    req["TMSI"] = settings.getContext().getTMSI();
    req["MSISDN_D"] = msisdn_d;
    req["SMS_ID"] = id;
    req["type"] = "SMSStatus";
    req["ENode"] = enodebId;
    send(req);
}

void Exchange::connect() {
    if (api.createConnection() != 0) {
        spdlog::info("Не удалось соединиться с сервером");
        return;
    }

    IN_ACTIVE = true;

    if (runThread.joinable()) { runThread.join(); }
    if (signalThread.joinable()) {signalThread.join(); }

    runThread = std::thread(&Exchange::run, this);
    signalThread = std::thread(&Exchange::signalWorker, this);

    if (!attach()) {
        onDisconnect();
    }
}

Exchange::Exchange(AppSettings& settings_): settings(settings_), api(settings_.getNetworkAddress()), worker(settings_, *this) {

}

Exchange::~Exchange() {
    onDisconnect();
    if (runThread.joinable() && runThread.get_id() != std::this_thread::get_id()) {
        runThread.join();
    }
    if (signalThread.joinable() && signalThread.get_id() != std::this_thread::get_id()) {
        signalThread.join();
    }
}
