cmake_minimum_required(VERSION 3.20)

include(FetchContent)

project(sms_center_project)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)
#=====================
# установка зависимостей

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3)
FetchContent_MakeAvailable(json)

FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.16.0)
FetchContent_MakeAvailable(spdlog)


# На гитхабе автора указано: the project has AGPL license for open source project and MIT license after purchasing it for 50$
# не подходит для коммерческой разработки, только для опенсорс
FetchContent_Declare(
        sqlite_orm
        GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
        GIT_TAG v1.9.1
)
FetchContent_MakeAvailable(sqlite_orm)

#=====================
# проверка установки зависимостей

find_package(PkgConfig REQUIRED)
pkg_check_modules(URING REQUIRED liburing)  # это надо скачать
find_package(SQLite3 REQUIRED)  # это тоже libsqlite3-dev

#===========================
# MNC-S

set(MNC_S_SOURCES
        MNC-S/network/src/UEConnection.cpp
        MNC-S/network/include/UEconnection.h
        model/Handler/RequestHandler.h
        MNC-S/network/src/Listener.cpp
        MNC-S/network/include/Listener.h
        model/Context/Context.cpp
        model/Context/Context.h
        MNC-S/ENODE/src/ENodeB.cpp
        MNC-S/ENODE/include/ENodeB.h
        MNC-S/ENODE/src/EnodeHandler.cpp
        MNC-S/ENODE/include/EnodeHandler.h
        model/ENode/EnodeConfig.h
        model/StatusCodes/StatusCodes.h
        MNC-S/MME/src/MME.cpp
        MNC-S/MME/include/MME.h
        MNC-S/MME/src/MMEHandler.cpp
        MNC-S/MME/include/MMEHandler.h
        MNC-S/XLR/model/Subscriber.h
        MNC-S/XLR/include/XLR.h
        MNC-S/XLR/src/XLR.cpp
        model/SMS/SMSRecord.h
        MNC-S/network/include/Sender.h
        MNC-S/Utils/Parser.cpp
        MNC-S/Utils/Parser.h
        MNC-S/Utils/JsonValidator.h
        MNC-S/ENODE/include/ENodeJSONSchemas.h
        MNC-S/MME/include/MMEJSONSchemas.h
        UE/include/Menu/MenuItems/DeactivateItem.h
)

set(MNC_S_LIBS
        ${URING_LIBRARIES}
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        sqlite_orm::sqlite_orm
        SQLite::SQLite3
)

add_executable(mnc_s MNC-S/mnc_s.cpp ${MNC_S_SOURCES})
target_link_libraries(mnc_s PRIVATE ${MNC_S_LIBS})

#================
# UE

set(UE_SOURCES

        UE/src/Settings/AppSettings.cpp
        UE/src/Settings/ArgParser.cpp
        UE/src/Menu/Menu.cpp
        UE/src/utils/utils.cpp
        UE/src/Network/NetworkAddress.cpp
        UE/src/Network/NetworkClient.cpp
        UE/src/Menu/MenuItem.cpp
        UE/include/Menu/MenuItem.h
        UE/include/Menu/MenuItems/QuitItem.h
        UE/src/Network/Exchange.cpp
        UE/include/Network/Exchange.h
        model/Context/Context.cpp
        model/Context/Context.h
        UE/include/Menu/MenuItems/SendSMSItem.h
        model/SMS/SMSRecord.h
        UE/src/Network/ExchangeHandler.cpp
        UE/include/Network/ExchangeHandler.h
        UE/include/Menu/MenuItems/PrintSMSItem.h
        UE/include/Menu/MenuItems/MoveItem.h
        UE/include/Menu/MenuItems/ActivateItem.h
)

add_executable(ue UE/UE.cpp ${UE_SOURCES})
target_link_libraries(ue PRIVATE spdlog::spdlog)
target_link_libraries(ue PRIVATE nlohmann_json::nlohmann_json)
