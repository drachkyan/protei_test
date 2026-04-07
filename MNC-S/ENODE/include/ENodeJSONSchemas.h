#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct RadioMeasureSchema {
    std::string type = "R";
    std::string IMSI;
    std::string TMSI;
    int x;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RadioMeasureSchema, type, IMSI, TMSI, x)

struct AuthResponseSchema {
    std::string type = "AR";
    std::string TMSI;
    std::string IMEI;
    int ENode;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AuthResponseSchema, type, TMSI, IMEI, ENode)

struct AttachRequestSchema {
    std::string type = "A";
    std::string IMEI;
    std::string IMSI;
    std::string MSISDN;
    int ENode;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AttachRequestSchema, type, IMEI, IMSI, MSISDN, ENode)

struct SendSMSSchema {
    std::string type = "M";
    std::string TMSI_S;
    std::string MSISDN_D;
    std::string SMS;
    int SMS_ID;
    int ENode;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SendSMSSchema, type, TMSI_S, MSISDN_D, SMS, SMS_ID, ENode)

struct SendSMSStatusSchema {
    std::string type = "SMSStatus";
    std::string MSISDN_D;
    std::string message_status;
    std::string TMSI;
    int SMS_ID;
    int ENode;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SendSMSStatusSchema, type, MSISDN_D, message_status, TMSI, SMS_ID, ENode)

struct DisconnectSchema {
    std::string type = "DC";
    std::string TMSI;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DisconnectSchema, type, TMSI)