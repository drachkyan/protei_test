#include "../include/EnodeHandler.h"
#include <spdlog/spdlog.h>
#include <cmath>
#include "../../../model/StatusCodes/StatusCodes.h"
#include "../include/ENodeB.h"
#include "../include/ENodeJSONSchemas.h"
#include "../../Utils/JsonValidator.h"

void EnodeHandler::initHandlersMap() {
    {
        handlersMap["R"] = &EnodeHandler::handleRadioMeasure;
        handlersMap["A"] = &EnodeHandler::handleAuth;
        handlersMap["AR"] = &EnodeHandler::handleUpdateLocation;
        handlersMap["M"] = &EnodeHandler::handleMessage;
        handlersMap["DC"] = &EnodeHandler::handleDisconnect;
        handlersMap["SMSStatus"] = &EnodeHandler::handleSMSStatus;
        handlersMap["H"] = &EnodeHandler::handleHandover;
    }
    {
        handlersInternalMap["SM"] = &EnodeHandler::handleSendMessage;
        handlersInternalMap["RS"] = &EnodeHandler::handleReceiveSMS;
        handlersInternalMap["SS"] = &EnodeHandler::handleSendSMS;
        handlersInternalMap["SMSStatus"] = &EnodeHandler::handleSendStatus;
    }

}

json EnodeHandler::requestMME(const json req, bool wait_flag) const {
    std::promise<json> promise;
    auto futureRes = promise.get_future();
    mme.push(MMETask{req,std::move(promise)});
    if (!wait_flag) {
        return {};
    }
    auto res = futureRes.get();
    return res;
}

json EnodeHandler::handleAuth(const json &req){

    auto schema = validate<AttachRequestSchema>(req);
    if (!schema) {
        spdlog::info("[ENODE{}] Пришел неверный запрос", config.id);
        return StatusCode::BAD_REQUEST_JSON;
    }
    spdlog::info("[ENODE{}] Запрос на авторизацию", config.id);
    json MMEreq {
            {"type", "A"},
            {"IMSI", schema->IMSI},
            {"IMEI", schema->IMEI},
            {"MSISDN", schema->MSISDN},
    };

    auto res = requestMME(MMEreq);
    if (!res.contains("TMSI")) {
        return StatusCode::BAD_REQUEST_JSON;
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
        spdlog::info("[ENODE{}] Получен пустой жсон", config.id);
        return {};
    }
    if (!req.contains("type")) {
        spdlog::info("[ENODE{}] JSON в неверном формате", config.id);
        return {};
    }
    std::string type = req["type"].get<std::string>();

    auto it = map.find(type);
    if (it == map.end()) {
        spdlog::error("[ENODE{}] неизвестный тип: {}", config.id, type);
        return {};
    }
    return (this->*it->second)(req);
}

void EnodeHandler::proccessSMS(const json &req) {
    auto SMS_ID = req["SMS_ID"].get<int>();
    auto text = req["SMS"].get<std::string>();

    SMSMessage SMS{SMS_ID, text, {}, {}};
    ENodes[config.id]->addSMStoSlot(req["TMSI_S"], req["MSISDN_D"], SMS);

    auto res = requestMME(req);

}


json EnodeHandler::handleRadioMeasure(const json &req) {
    // spdlog::info("[ENODE{}] Запрос силы сигнала", config.id);   // -- слишком часто идет
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
    auto schema = validate<AuthResponseSchema>(req);
    if (!schema) {
        spdlog::info("[ENODE{}] Пришел неверный запрос", config.id);
        return StatusCode::BAD_REQUEST_JSON;
    }
    spdlog::info("[ENODE{}] Запрос на прикрепление", config.id);
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
    spdlog::info("[ENODE{}] пересылка сообщения на другую базовую станцию", config.id);
    const auto& enodeD_ID = req["ENodeD"].get<int>();
    const std::string tmsi_d = req["TMSI_D"].get<std::string>();
    const std::string tmsi_s = req["TMSI_S"].get<std::string>();
    const std::string msisdn_s = req["MSISDN_S"].get<std::string>();

    if (!ENodes.contains(enodeD_ID)) {
        spdlog::info("[ENODE{}] Не найдена базовая станция", config.id);
    }

    auto enodeD = ENodes[enodeD_ID];
    auto enodeS = ENodes[config.id];

    auto SMS = enodeS->getSMStoSend(tmsi_s, req["MSISDN_D"]);
    if (!SMS) {
        spdlog::info("[ENODE{}] Собщение не найдено", config.id);
        return StatusCode::SERVER_ERROR_JSON;
    }
    SMS->msisdn_src = msisdn_s;
    SMS->tmsi_dst = tmsi_d;

    json j_sms = *SMS;

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
    auto schema = validate<SendSMSSchema>(req);
    if (!schema) {
        spdlog::info("[ENODE{}] Пришел неверный запрос", config.id);
        return StatusCode::BAD_REQUEST_JSON;
    }
    spdlog::info("[ENODE{}] получено сообщение", config.id);
    std::thread(&EnodeHandler::proccessSMS, this, req).detach();
    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleReceiveSMS(const json &req) {
    auto enode = ENodes[config.id];
    auto SMS = req["SMS"].get<SMSMessage>();
    enode->receiveSMS(SMS);
    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleSendSMS(const json &req) {
    auto TMSI_D = req["TMSI_D"].get<std::string>();
    auto SMS = ENodes[config.id]->getSMSbyTMSI(TMSI_D);
    spdlog::info("[ENODE{}] Отправка СМС пользователю {}, текст смс [ {} ]", config.id, TMSI_D, SMS.text);
    json sms_json = SMS;
    sms_json["type"] = "SMS";
    sender.sendByTMSI(TMSI_D, sms_json);
    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleSMSStatus(const json &req) {
    auto schema = validate<SendSMSStatusSchema>(req);
    if (!schema) {
        spdlog::info("[ENODE{}] Пришел неверный запрос", config.id);
        return StatusCode::BAD_REQUEST_JSON;
    }
    spdlog::info("[ENODE{}] Пришел статус сообщения", config.id);

    std::thread([this, req]() {
        auto res = requestMME(req);
        if (!res.contains("TMSI")) {
            return;
        };
        auto TMSI = res["TMSI"].get<std::string>();
        res.erase("TMSI");

        sender.sendByTMSI(TMSI, res);

    }).detach();



    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleSendStatus(const json &req) {
    auto schema = validate<SendSMSStatusSchema>(req);
    if (!schema) {
        spdlog::info("[ENODE{}] Пришел неверный запрос", config.id);
        return StatusCode::BAD_REQUEST_JSON;
    }
    spdlog::info("[ENODE{}] Запрос на статус сообщения", config.id);
    ENodes[config.id]->deleteSMSfromSlot(schema->TMSI, schema->MSISDN_D);
    sender.sendByTMSI(schema->TMSI, req);
    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleDisconnect(const json &req) {
    auto schema = validate<DisconnectSchema>(req);
    if (!schema) {
        spdlog::info("[ENODE{}] Пришел неверный запрос", config.id);
        return StatusCode::BAD_REQUEST_JSON;
    }
    spdlog::info("[ENODE{}] Отключаем клиента", config.id);
    auto TMSI = schema->TMSI;

    ENodes[config.id]->releaseSlot(TMSI);


    auto a = requestMME(req);

    return StatusCode::SUCCESS_JSON;
}

json EnodeHandler::handleHandover(const json &req) {
    auto schema = validate<HandoverRequestSchema>(req);
    if (!schema) {
        spdlog::info("[ENODE{}] Пришел неверный запрос", config.id);
        return StatusCode::BAD_REQUEST_JSON;
    }
    spdlog::info("[ENODE{}] Handover процедура");
    auto slotToHandover = ENodes[config.id]->detachSlot(schema->TMSI);
    if (!slotToHandover) {
        spdlog::info("[ENODE{}] Слот не найден", config.id);
        return StatusCode::BAD_REQUEST_JSON;
    }
    ENodes[schema->ENode_D]->handover(schema->TMSI, *slotToHandover);
    auto MMEreq = req;
    MMEreq.erase("ENode");
    auto res = requestMME(MMEreq, false);
    return StatusCode::SUCCESS_JSON;
}

EnodeHandler::EnodeHandler(ENodeConfig &config_, MME &mme_, std::unordered_map<int, ENodeB *> &ENodes_, Sender& sender_)
                        : config(config_), mme(mme_), ENodes(ENodes_), sender(sender_) {
    initHandlersMap();
}
