//
// Created by vova on 04.04.2026.
//

#include "../include/MMEHandler.h"
#include <spdlog/spdlog.h>

#include "../../../model/StatusCodes/StatusCodes.h"
#include "../../ENODE/include/ENodeB.h"

void MMEHandler::initHandlersMap() {
    handlersMap["A"] = &MMEHandler::handleAttach;
    handlersMap["UL"] = &MMEHandler::handleUpdateLocation;
    handlersMap["M"] = &MMEHandler::handleSMS;
    handlersMap["DC"] = &MMEHandler::handleDisconnect;
}

json MMEHandler::handleAttach(const json &req) {

    if (!req.contains("IMSI")) {
        spdlog::info("[MME] Нет IMSI в запросе");
        return StatusCode::NOT_FOUND_JSON;
    }
    auto TMSI = generateTMSI(TMSI_script);
    xlr.updateTmsi(req["IMSI"], TMSI);
    auto sub = xlr.findByImsi(req["IMSI"]);

    if (!sub) {
        spdlog::info("[MME] Неизвестный абонент");
        return StatusCode::NOT_FOUND_JSON;
    }


    auto res = StatusCode::SUCCESS_JSON;
    res["TMSI"] = TMSI;
    return res;
}

json MMEHandler::handleUpdateLocation(const json &req) {

    if (!req.contains("TMSI")) {
        spdlog::info("[MME] Нет TMSI в запросе");
        return StatusCode::NOT_FOUND_JSON;
    }
    if (!req.contains("ENode")) {
        spdlog::info("[MME] Не выбрана базовая станция");
        return StatusCode::NOT_FOUND_JSON;
    }

    const auto ENodeID = req["ENode"].get<int>();
    const auto& TMSI = req["TMSI"];
    if (!ENodes[ENodeID]->reserveSlot(TMSI)) {
        spdlog::info("[MME] Не получилось создать буфер ENode");
        return StatusCode::SERVER_ERROR_JSON;
    }

    xlr.updateEnodeB(TMSI, ENodeID);

    return StatusCode::SUCCESS_JSON;;
}

std::string MMEHandler::generateTMSI(std::string& scriptPath){
    const std::string cmd = "python3 " + scriptPath;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        spdlog::error("[MME] Не удалось запустить скрипт: {}", scriptPath);
        return "";
    }
    char buffer[64];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);

    std::erase(result, '\n');
    return result;
}

json MMEHandler::sendSMS(ENodeB* ENodeS, int enodeD, std::string TMSI_D, std::string TMSI_S, std::string MSISDN_S, std::string MSISDN_D) {
    spdlog::info("[MME] пересылка сообщения на базовую станцию");
    std::promise<json> promise;

    json req = {
        {"type", "SM"},
        {"MSISDN_S", MSISDN_S},
        {"TMSI_D", TMSI_D},
        {"TMSI_S", TMSI_S},
        {"ENodeD", enodeD},
        {"MSISDN_D", MSISDN_D}
    };

    ENodeS->pushInternalTask(Task{req, std::move(promise)});
    spdlog::info("[MME] задача отправлена в eNode-B");
    return StatusCode::SUCCESS_JSON;
}

std::optional<Subscriber> MMEHandler::handleWaitAbonent(std::string MSISDN_D) {

    spdlog::info("Ожидание появления абонента");
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);

    while (std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        auto rec = xlr.findByMsisdn(MSISDN_D);
        if (rec && rec->enodeb_id > 0) {
            return rec;
        }

    }
    return std::nullopt;

}


json MMEHandler::handleSMS(const json &req) {
    spdlog::info("[MME] Проверка наличия абонента в сети");

    const auto TMSI_S = req["TMSI_S"].get<std::string>();
    const auto MSISDN_D = req["MSISDN_D"].get<std::string>();
    const auto enodeS = ENodes[req["ENode"].get<int>()];
    const auto sender = xlr.findByTmsi(TMSI_S);

    if (!sender) {
        spdlog::info("[MME] Ошибка - отправитель не зарегестрирован в сети");
        return StatusCode::NOT_FOUND_JSON;
    }
    const auto MSISDN_S = sender->msisdn;
    auto rec = xlr.findByMsisdn(MSISDN_D);

    if (!rec) {
        spdlog::info("[MME] Ошибка - Абонента не существует в сети");
        return StatusCode::NOT_FOUND_JSON;
    }

    if (rec->enodeb_id < 0) {
        rec = handleWaitAbonent(MSISDN_D);
    }
    if (!rec) {
        spdlog::info("[MME] TTL SMS истёк, абонент {} недоступен", MSISDN_D);
        return StatusCode::NOT_FOUND_JSON;
    }

    std::thread(&MMEHandler::sendSMS,this, enodeS, rec->enodeb_id, rec->tmsi, TMSI_S, MSISDN_S, rec->msisdn).detach();

    auto res = StatusCode::SUCCESS_JSON;

    return res;
}

json MMEHandler::handleDisconnect(const json &req) {
    spdlog::info("[MME] Отключение клиента");

    if (!req.contains("TMSI")) {
        return StatusCode::NOT_FOUND_JSON;
    }

    auto TMSI = req["TMSI"].get<std::string>();
    xlr.clearTmsi(TMSI);

    return StatusCode::SUCCESS_JSON;
}

MMEHandler::MMEHandler(std::string path, XLR& xlr_, std::unordered_map<int, ENodeB*>& enodes_)
        : TMSI_script(std::move(path)), xlr(xlr_), ENodes(enodes_) {

    initHandlersMap();
}

json MMEHandler::handle(const json &req) {
    json res{};
    if (req.empty()) {
        spdlog::info("[MME] Получен пустой жсон");
        return res;
    }
    if (!req.contains("type")) {
        spdlog::info("[MME] JSON в неверном формате");
        return res;
    }
    std::string type = std::move(req["type"].get<std::string>());

    if (handlersMap.find(type) == handlersMap.end()) {
        spdlog::error("[MME] неизвестный тип: {}", type);
        return res;
    }
    auto handler = handlersMap[type];
    res = (this->*handler)(req);
    return res;
}
