#include "Parser.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

EPCConfig parseEPC(const std::string& path) {
    std::ifstream file(path);
    json j = json::parse(file);

    return EPCConfig{
        j["ttl_connections"].get<int>(),
        j["hlr_format"].get<std::string>(),
        j["hlr_provider"].get<std::string>(),
        j["hlr_path"].get<std::string>(),
        j["ttl_ue_context"].get<int>(),
        j["ttl_sms"].get<int>(),
        j["tmsi_script"].get<std::string>(),
        j["xdr_format"].get<std::string>(),
        j["xdr_provider"].get<std::string>(),
        j["xdr_path"].get<std::string>()
    };
}

std::vector<BaseConfig> parseBases(const std::string& path) {
    std::ifstream file(path);
    json j = json::parse(file);

    std::vector<BaseConfig> enodes;
    for (auto& e : j["enodeb"]) {
        enodes.push_back({
            e["id"].get<int>(),
            e["x"].get<double>(),
            e["power"].get<double>(),
            e["radius"].get<double>(),
            e["max_connections"].get<int>(),
            e["buffer_size"].get<int>()
        });
    }
    return enodes;
}