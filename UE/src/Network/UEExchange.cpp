#include "../../include/Network/UEExchange.h"
#include "../../../model/StatusCodes/StatusCodes.h"

json UEExchange::handleAuth() {
    json req = {
        {"type", "A"},
        {"IMSI", settings.getContext().getIMSI()},
        {"IMEI", settings.getContext().getIMEI()},
        {"MSISDN", settings.getContext().getMSISDN()},
        {"ENode", enodebId}
    };
    json res{};
    api.sendRecv(req,res);
    return res;
}

json UEExchange::handleAttachResponse() {
    json req = {
        {"type", "AR"},
        {"TMSI", settings.getContext().getTMSI()},
        {"IMEI", settings.getContext().getIMEI()},
        {"ENode", enodebId}
    };
    json res{};
    api.sendRecv(req,res);
    return res;
}

json UEExchange::radioMeasure() {
    json req{}, res{};
    req["type"] = "R";

    if (!settings.getContext().getTMSI().empty()) {
        req["TMSI"] = settings.getContext().getTMSI();
    }else {
        req["IMSI"] = settings.getContext().getIMSI();
    }

    req["pos"] = settings.getContext().getX();
    api.sendRecv(req, res);
    return res;
}

void UEExchange::attach() {
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

    auto auth_res = handleAuth();

    if (!auth_res.contains("TMSI")) {
        spdlog::info("Не удалось авторизироваться к станции");
        return;
    }
    settings.getContext().setTMSI(auth_res["TMSI"]);

    auto res = handleAttachResponse();
    if (static_cast<int>(StatusCode::SUCCESS) != res["status"]) {
        spdlog::info("Возникла ошибка подключения");
        return;
    }

    IN_ACTIVE = true;
    spdlog::info("Подключены к станции");
}

void UEExchange::connect() {
    if (api.createConnection() != 0) {
        spdlog::info("Не удалось соединиться с сервером");
        return;
    }
    attach();
}

UEExchange::UEExchange(AppSettings& settings_): settings(settings_), api(settings_.getNetworkAddress()) {

}
