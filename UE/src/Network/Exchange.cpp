#include "../../include/Network/Exchange.h"
#include "../../../model/StatusCodes/StatusCodes.h"

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
            spdlog::info("Сервер отключился");
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

void Exchange::signalWorker() {
    while (IN_ACTIVE) {
        std::unique_lock lock(signalMtx);
        signalCv.wait_for(lock, std::chrono::seconds(5));
        spdlog::info("Запрос станций");
        if (!IN_ACTIVE) break;

        auto res = radioMeasure();
        if (!res.contains("ENodes") || res["ENodes"].empty()) {
            spdlog::info("Нет сигнала - отключение");
            onDisconnect();
            return;
        }

        auto& enodes = res["ENodes"];
        auto best = std::ranges::max_element(enodes,
            [](const json& a, const json& b) {
                return a["power"].get<double>() < b["power"].get<double>();
            });

        auto bestId = (*best)["ENode"].get<int>();

        if (bestId != enodebId) {
            spdlog::info("Handover на eNode-B {}", bestId);
        }
    }
}

void Exchange::onDisconnect() {
    IN_ACTIVE = false;
    signalCv.notify_all();
    api.close();
}

void Exchange::attach() {
    auto enodesPower = radioMeasure();

    if (!enodesPower.contains("ENodes") || enodesPower["ENodes"].empty()) {
        spdlog::error("Нет доступных базовых станций");
        return;
    }

    auto& enodes = enodesPower["ENodes"];
    auto best = std::ranges::max_element(enodes,
                                         [](const json& a, const json& b) {
                                             return a["power"].get<double>() < b["power"].get<double>();
                                         });

    enodebId = (*best)["ENode"].get<int>();
    double bestPower = (*best)["power"].get<double>();

    spdlog::info("Лучшая станция: {} сигнал: {}", enodebId, bestPower);

    auto auth_res = handleAttachRequest();

    if (!auth_res.contains("TMSI")) {
        spdlog::info("Не удалось авторизироваться к станции");
        return;
    }
    settings.getContext().setTMSI(auth_res["TMSI"]);

    auto res = handleAuthResponse();
    if (static_cast<int>(StatusCode::SUCCESS) != res["status"]) {
        spdlog::info("Возникла ошибка подключения");
        return;
    }

    IN_ACTIVE = true;
    spdlog::info("Подключены к станции");
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
    spdlog::info("[EX] Начало работы");
    IN_ACTIVE = true;
    runThread = std::thread(&Exchange::run, this);
    signalThread = std::thread(&Exchange::signalWorker, this);

    attach();
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
