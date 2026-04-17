#pragma once

#include <string>

struct Subscriber {
    std::string msisdn;
    std::string imei;
    std::string imsi;
    std::string tmsi;
    int enodeb_id = -1;
};