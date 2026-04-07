#pragma once
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct EPCConfig {
    int ttl_connections;
    std::string hlr_format;
    std::string hlr_provider;
    std::string hlr_path;
    int ttl_ue_context;
    int ttl_sms;
    std::string tmsi_script;
    std::string xdr_format;
    std::string xdr_provider;
    std::string xdr_path;
};

struct BaseConfig {
    int id;
    double x;
    double power;
    double radius;
    int max_connections;
    int buffer_size;
};

EPCConfig parseEPC(const std::string& path);

std::vector<BaseConfig> parseBases(const std::string& path);