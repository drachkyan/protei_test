#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct AttachSchema {
    std::string type = "A";
    std::string IMEI;
    std::string IMSI;
    std::string MSISDN;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AttachSchema, type, IMEI, IMSI, MSISDN)

struct SendSMSSchemaMME {
    std::string type = "M";
    std::string TMSI_S;
    std::string MSISDN_D;
    std::string SMS;
    int SMS_ID;
    int ENode;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SendSMSSchemaMME, type, TMSI_S, MSISDN_D, SMS, SMS_ID, ENode)

struct SendSMSStatusSchemaMME {
    std::string type = "SMSStatus";
    std::string MSISDN_D;
    std::string message_status;
    std::string TMSI;
    int SMS_ID;
    int ENode;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SendSMSStatusSchemaMME, type, MSISDN_D, message_status, TMSI, SMS_ID, ENode)

struct UpdateLocationSchema {
    std::string type = "UL";
    std::string TMSI;
    std::string IMEI;
    int ENode;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpdateLocationSchema, type, TMSI, IMEI, ENode)

struct DisconnectSchemaMME {
    std::string type = "DC";
    std::string TMSI;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DisconnectSchemaMME, type, TMSI)

struct HandoverSchemaMME {
    std::string type = "H";
    std::string TMSI;
    int ENode_D;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HandoverSchemaMME, type, TMSI, ENode_D)