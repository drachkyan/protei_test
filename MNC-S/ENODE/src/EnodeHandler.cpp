#include "../include/EnodeHandler.h"
#include <spdlog/spdlog.h>
#include <cmath>
#include "../../../model/StatusCodes/StatusCodes.h"
#include "../include/ENodeB.h"

void EnodeHandler::initHandlersMap() {
    {
        handlersMap["R"] = &EnodeHandler::handleRadioMeasure;
        handlersMap["A"] = &EnodeHandler::handleAuth;
        handlersMap["AR"] = &EnodeHandler::handleUpdateLocation;
        handlersMap["M"] = &EnodeHandler::handleMessage;
    }
    {
        handlersInternalMap["SM"] = &EnodeHandler::handleSendMessage;
        handlersInternalMap["RS"] = &EnodeHandler::handleReceiveSMS;
        handlersInternalMap["SS"] = &EnodeHandler::handleSendSMS;
    }


}

json EnodeHandler::requestMME(const json &req) const {
    std::promise<json> promise;
    auto futureRes = promise.get_future();
    mme.push(MMETask{req,std::move(promise)});
    auto res = futureRes.get();
    return res;
}

json EnodeHandler::handleAuth(const json &req){
    json MMEreq {
            {"type", "A"},
            {"IMSI", req["IMSI"]},
            {"IMEI", req["IMEI"]},
            {"MSISDN", req["MSISDN"]}
    };
    auto res = requestMME(MMEreq);
    if (!res.contains("TMSI")) {
        spdlog::info("Произошла ошибка на сервере");
        return StatusCode::SERVER_ERROR;
    }
    return res;
}

json EnodeHandler::handle(const json& req) {
    return dispatch(req, handlersMap);
}

json EnodeHandler::handleInternalTask(const json& req) {
    return dispatch(req, handlersInternalMap);
}

json EnodeHandler::dispatch(const json& req, const std::unordered_map<std::string, HandlerPtr>& map) {
    if (req.empty()) {
        spdlog::info("[ENODE] Получен пустой жсон");
        return {};
    }
    if (!req.contains("type")) {
        spdlog::info("[ENODE] JSON в неверном формате");
        return {};
    }
    std::string type = req["type"].get<std::string>();

    auto it = map.find(type);
    if (it == map.end()) {
        spdlog::error("[ENODE] неизвестный тип: {}", type);
        return {};
    }
    return (this->*it->second)(req);
}

void EnodeHandler::proccessSMS(const json &req) {
    spdlog::info("{}", req.dump());
    auto SMS_ID = req["SMS_ID"].get<int>();
    auto text = req["SMS"].get<std::string>();

    SMSMessage SMS{SMS_ID, text, {}, {}};
    ENodes[config.id]->addSMStoSlot(req["TMSI_S"], req["MSISDN_D"], SMS);

    auto res = requestMME(req);

}


json EnodeHandler::handleRadioMeasure(const json &req) {
    double pos = req["pos"].get<double>();
    json res{};
    double coef = 1 - std::abs(config.x - pos)/config.radius;
    if (coef < 0) {
        return res;
    }
    res["power"] = coef;
    res["ENode"] = config.id;
    return res;
}


json EnodeHandler::handleUpdateLocation(const json &req) {
    auto newReq = req;
    newReq["type"] = "UL";
    auto res = requestMME(newReq);
    if (!res.contains("status")) {
        res["status"] = StatusCode::SERVER_ERROR;
        return res ;
    }

    return res;

}

json EnodeHandler::handleSendMessage(const json &req) {
    spdlog::info("[ENODE] пересылка сообщения на другую базовую станцию");
    spdlog::info("{}", req.dump());
    const auto& enodeD_ID = req["ENodeD"].get<int>();
    const std::string tmsi_d = req["TMSI_D"].get<std::string>();
    const std::string tmsi_s = req["TMSI_S"].get<std::string>();
    const std::string msisdn_s = req["MSISDN_S"].get<std::string>();

    if (!ENodes.contains(enodeD_ID)) {
        spdlog::info("[ENODE] Не найдена базовая станция");
    }

    auto enodeD = ENodes[enodeD_ID];
    auto enodeS = ENodes[config.id];

    auto SMS = enodeS->getSMStoSend(tmsi_s, req["MSISDN_D"]);

    SMS.msisdn_src = msisdn_s;
    SMS.tmsi_dst = tmsi_d;

    json j_sms = SMS;
    spdlog::info("Сообщение: {}", j_sms.dump());
    json newReq = {
        {"type", "RS"},
        {"SMS", j_sms}
    };

    std::promise<json> promise;
    auto fut = promise.get_future();
    enodeD->pushInternalTask(Task{newReq, std::move(promise)});
    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleMessage(const json &req) {
    spdlog::info("[ENODE] получено сообщение");
    std::thread(&EnodeHandler::proccessSMS, this, req).detach();
    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleReceiveSMS(const json &req) {
    auto enode = ENodes[config.id];
    auto SMS = req["SMS"].get<SMSMessage>();
    spdlog::info("{}", SMS.tmsi_dst);
    enode->receiveSMS(SMS);
    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleSendSMS(const json &req) {
    spdlog::info("{}", req.dump());
    auto TMSI_D = req["TMSI_D"].get<std::string>();
    auto SMS = ENodes[config.id]->getSMSbyTMSI(TMSI_D);
    spdlog::info("[ENODE] Отправка СМС пользователю {}, текст смс [ {} ]", TMSI_D, SMS.text);
    json sms_json = SMS;
    sms_json["type"] = "SMS";
    sender.sendByTMSI(TMSI_D, sms_json);
    return StatusCode::SUCCESS_JSON;
}

EnodeHandler::EnodeHandler(ENodeConfig &config_, MME &mme_, std::unordered_map<int, ENodeB *> &ENodes_, Sender& sender_)
                        : config(config_), mme(mme_), ENodes(ENodes_), sender(sender_) {
    initHandlersMap();
}
