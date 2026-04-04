#pragma once

#include <optional>
#include "../model/Subscriber.h"
#include <sqlite_orm/sqlite_orm.h>

using namespace sqlite_orm;

inline auto makeStorage(const std::string& path) {
    using namespace sqlite_orm;
    return make_storage(path,
        make_table("subscribers",
            make_column("msisdn", &Subscriber::msisdn, primary_key()),
            make_column("imei", &Subscriber::imei),
            make_column("imsi", &Subscriber::imsi, unique()),
            make_column("tmsi", &Subscriber::tmsi),
            make_column("enodeb_id", &Subscriber::enodeb_id)
        )
    );
}

using DB = decltype(makeStorage(""));  //  тут на этапе компиляции вычисляется

class XLR {
    DB db;

public:
    XLR(const std::string& path);
    void insert(const Subscriber& s);
    std::optional<Subscriber> findByImsi(const std::string& imsi);
    std::optional<Subscriber> findByTmsi(const std::string& tmsi);
    void updateTmsi(const std::string& imsi, const std::string& tmsi, int enodebId);
    void clearTmsi(const std::string& tmsi);
};