cmake_minimum_required(VERSION 4.1)

include(FetchContent)

project(sms_center_project)

set(CMAKE_CXX_STANDARD 20)

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

#=====================
# проверка установки зависимостей

find_package(PkgConfig REQUIRED)
pkg_check_modules(URING REQUIRED liburing)


add_executable(sms_center_project main.cpp
        transport/src/TransportWorker.cpp
        transport/include/TransportWorker.h)
