#include "../../include/Settings/ArgParser.h"

void ArgParser::initFuncMap() {
    funcMap["-p"] = [this](const char* arg) { this->port = arg; };
    funcMap["-i"] = [this](const char* arg) { this->addr = arg; };
    funcMap["-imei"] = [this](const char* arg) { this->imei = arg; };
    funcMap["-imsi"] = [this](const char* arg) { this->imsi = arg; };
    funcMap["-msisdn"] = [this](const char* arg) { this->msisdn = arg; };
    funcMap["-x"] = [this](const char* arg) { this->x = std::stoi(arg); };
}

void ArgParser::parse(int argc, char *argv[]) {
    if (argc % 2 == 0 || argc < 3) {
        spdlog::error("Неверное число аргументов");
        return;
    }

    for (int i = 1; i < argc; i += 2) {
        auto it = funcMap.find(argv[i]);
        if (it != funcMap.end()) {
            it->second(argv[i + 1]);
        } else {
            spdlog::error("Неизвестный флаг: {}", argv[i]);
        }
    }
}


