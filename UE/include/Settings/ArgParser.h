#pragma once
#include <string>
#include "AppSettings.h"
#include "../../../model/Context/Context.h"



class ArgParser {
    using func_type = std::function<void(char*)>;

    std::unordered_map<std::string, func_type> funcMap;

    std::string port;
    std::string addr;
    std::string imei;
    std::string imsi;
    std::string msisdn;
    int x = 0;

    void initFuncMap();

public:
    ArgParser() { this->initFuncMap(); }

    void parse(int argc, char* argv[]);


    [[nodiscard]] AppSettings build() const {

        return {
            NetworkAddress(port, addr),
            Context(imsi, imei, msisdn, x)
        };
    }
};

