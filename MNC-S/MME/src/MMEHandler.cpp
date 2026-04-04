//
// Created by vova on 04.04.2026.
//

#include "../include/MMEHandler.h"
#include <spdlog/spdlog.h>

#include "../../../model/StatusCodes/StatusCodes.h"

void MMEHandler::initHandlersMap() {
    handlersMap["A"] = &MMEHandler::handleAttach;
    handlersMap["UL"] = &MMEHandler::handleUpdateLocation;
}

json MMEHandler::handleAttach(const json &req) {

    if (!req.contains("IMSI")) {
        spdlog::info("Нет IMSI в запросе");
        return StatusCode::NOT_FOUND_JSON;
    }

    auto sub = xlr.findByImsi(req["IMSI"]);

    if (!sub) {
        spdlog::info("Неизвестный абонент");
        return StatusCode::NOT_FOUND_JSON;
    }

    auto TMSI = generateTMSI(TMSI_script);
    auto res = StatusCode::SUCCESS_JSON;
    res["TMSI"] = TMSI;
    return res;
}

json MMEHandler::handleUpdateLocation(const json &req) {
    json res = StatusCode::SUCCESS_JSON;
    spdlog::info("[MME] Пришло на апдейт локейшен: {}", req.dump());

    return res;
}

std::string MMEHandler::generateTMSI(std::string& scriptPath){
    std::string cmd = "python3 " + scriptPath;
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

    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
}

MMEHandler::MMEHandler(std::string path, XLR& xlr_): TMSI_script(std::move(path)), xlr(xlr_) { initHandlersMap(); }

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
