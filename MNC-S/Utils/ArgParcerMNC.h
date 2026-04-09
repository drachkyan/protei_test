#pragma once

#include <string>
#include <functional>
#include <unordered_map>

class ArgParserMNC {
    using func_type = std::function<void(char*)>;

    std::unordered_map<std::string, func_type> funcMap;

    int port = 0;
    std::string basesJsonPath{};
    std::string epcJsonPath{};

    void initFuncMap();

public:
    ArgParserMNC() { this->initFuncMap(); }

    bool isValid() const {
        if ((port <= 0) || basesJsonPath.empty() || epcJsonPath.empty()) {
            return false;
        }
        if (basesJsonPath.empty()) {
            return false;
        }
        if (epcJsonPath.empty()) {
            return false;
        }
        return true;
    }

    int getPort() const { return this->port; }
    std::string getBasesJsonPath() const { return this->basesJsonPath; }
    std::string getEpcJsonPath() const { return this->epcJsonPath; }

    void parse(int argc, char* argv[]);

};