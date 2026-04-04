#include "../include/EnodeHandler.h"
#include <spdlog/spdlog.h>
#include <cmath>
#include "../../../model/StatusCodes/StatusCodes.h"

void EnodeHandler::initHandlersMap() {
    handlersMap["R"] = &EnodeHandler::handleRadioMeasure;
    handlersMap["A"] = &EnodeHandler::handleAuth;
    handlersMap["AR"] = &EnodeHandler::handleUpdateLocation;
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
    return res;
}



json EnodeHandler::handle(const json& req) {
    json res{};
    if (req.empty()) {
        spdlog::info("Получен пустой жсон");
        return res;
    }
    if (!req.contains("type")) {
        spdlog::info("JSON в неверном формате");
        return res;
    }
    std::string type = std::move(req["type"].get<std::string>());

    if (handlersMap.find(type) == handlersMap.end()) {
        spdlog::error("неизвестный тип: {}", type);
        return res;
    }
    auto handler = handlersMap[type];
    res = (this->*handler)(req);
    return res;
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
    if (res.find("status") == res.end()) {
        res["status"] = StatusCode::SERVER_ERROR;
        return res ;
    }

    return res;

}

EnodeHandler::EnodeHandler(ENodeConfig& config_, MME& mme_): config(config_), mme(mme_) {
    initHandlersMap();
}
