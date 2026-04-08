#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "spdlog/spdlog.h"
#include "../MNC-S/ENODE/include/ENodeB.h"
#include "../UE/include/Network/NetworkAddress.h"
#include "../UE/include/Network/Exchange.h"
#include "../UE/include/utils/utils.h"
#include "../UE/include/Settings/ArgParser.h"
#include "../MNC-S/XLR/include/XLR.h"


using json = nlohmann::json;



TEST(UtilsTest, ToLowerCaseTest) {
    std::string data = "HeLLo WoRLD 123";
    toLowerCase(data);
    EXPECT_EQ(data, "hello world 123");
    
    std::string empty = "";
    toLowerCase(empty);
    EXPECT_EQ(empty, "");
}

TEST(UtilsTest, HashFunctionTest) {
    const char* str1 = "test";
    const char* str2 = "test";
    const char* str3 = "different";
    
    EXPECT_EQ(hash(str1), hash(str2));
    EXPECT_NE(hash(str1), hash(str3));
}

TEST(NetworkAddressTest, ValidIpAndPort) {
    NetworkAddress addr("8080", "127.0.0.1");
    EXPECT_TRUE(addr.isCorrect());
    EXPECT_EQ(addr.getPort(), 8080);
    
    std::stringstream ss;
    ss << addr;
    EXPECT_EQ(ss.str(), "127.0.0.1:8080");
}

TEST(NetworkAddressTest, InvalidIp) {
    NetworkAddress addr("8080", "256.0.0.1");
    EXPECT_FALSE(addr.isCorrect());
    
    NetworkAddress addr2("8080", "127.0.1");
    EXPECT_FALSE(addr2.isCorrect());
}

TEST(NetworkAddressTest, InvalidPort) {
    NetworkAddress addr("70000", "127.0.0.1");
    EXPECT_FALSE(addr.isCorrect());
    
    NetworkAddress addr2("-1", "127.0.0.1");
    EXPECT_FALSE(addr2.isCorrect());
}


TEST(ArgParserTest, ValidArguments) {
    ArgParser parser;
    
    char* argv[] = {
        (char*)"program",
        (char*)"-i", (char*)"192.168.1.1",
        (char*)"-p", (char*)"5000",
        (char*)"-imei", (char*)"123456789"
    };
    int argc = 7;
    
    parser.parse(argc, argv);
    
    EXPECT_EQ(parser.getAddr(), "192.168.1.1");
    EXPECT_EQ(parser.getPort(), "5000");
    EXPECT_EQ(parser.getImei(), "123456789");
}


TEST(XLRTest, DatabaseOperations) {
    std::string testDb = "test_xlr.db";
    XLR xlr(testDb);
    
    Subscriber sub;
    sub.imsi = "12345";
    sub.msisdn = "79991234567";
    sub.tmsi = "";
    sub.enodeb_id = -1;
    
    xlr.insert(sub);
    
    auto found = xlr.findByImsi("12345");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->msisdn, "79991234567");
    
    xlr.updateTmsi("12345", "TMSI_NEW");
    auto foundTmsi = xlr.findByTmsi("TMSI_NEW");
    ASSERT_TRUE(foundTmsi.has_value());
    EXPECT_EQ(foundTmsi->imsi, "12345");
    
    std::remove(testDb.c_str());
}


TEST(ExchangeTest, JSONPayloadFormat) {
    
    json req = {
        {"type", "A"},
        {"IMSI", "123"},
        {"IMEI", "456"},
        {"MSISDN", "789"},
        {"ENode", 1}
    };
    
    EXPECT_EQ(req["type"], "A");
    EXPECT_TRUE(req.contains("IMSI"));
    EXPECT_TRUE(req["ENode"].is_number());
}


TEST(MMEHandlerTest, HandleInvalidJSON) {

    XLR xlr("dummy.db");
    std::unordered_map<int, ENodeB*> enodes;
    MMEHandler handler("gen_tmsi.py", xlr, enodes);
    
    json empty_req = json::object();
    json res = handler.handle(empty_req);

    EXPECT_TRUE(res.empty());
    
    std::remove("dummy.db");
}

int main(int argc, char **argv) {
    spdlog::set_level(spdlog::level::off);
    spdlog::set_pattern("");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}