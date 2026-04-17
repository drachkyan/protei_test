#include "ArgParcerMNC.h"
#include <spdlog/spdlog.h>

void ArgParserMNC::initFuncMap() {
    funcMap["-p"] = [this](const char* arg) { this->port = std::stoi(arg); };
    funcMap["-epc"] = [this](const char* arg) { this->epcJsonPath = arg; };
    funcMap["-bases"] = [this](const char* arg) { this->basesJsonPath = arg; };
}

void ArgParserMNC::parse(int argc, char *argv[]) {
    if (argc % 2 == 0 || argc < 3) {
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

