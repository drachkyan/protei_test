#pragma once

#include <string>

#include "../StatusCodes/StatusCodes.h"

struct SMSRecord {
    std::string msisdn;
    std::string text;
    MessageStatus status;
    std::chrono::system_clock::time_point time;
};
