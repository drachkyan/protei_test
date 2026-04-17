#include "../include/XLR.h"
#include <sqlite_orm/sqlite_orm.h>

using namespace sqlite_orm;


XLR::XLR(const std::string& path): db(makeStorage(path)) {
    std::lock_guard lock(mtx);
    db.sync_schema();

    db.update_all(
        set(c(&Subscriber::enodeb_id) = -1)
    );
}

void XLR::insert(const Subscriber &s) {
    std::lock_guard lock(mtx);
    db.replace(s);
}

std::optional<Subscriber> XLR::findByImsi(const std::string &imsi) {
    std::lock_guard lock(mtx);
    auto results = db.get_all<Subscriber>(where(c(&Subscriber::imsi) == imsi));
    if (results.empty()) return std::nullopt;
    return results.front();
}

std::optional<Subscriber> XLR::findByTmsi(const std::string& tmsi) {
    std::lock_guard lock(mtx);
    auto results = db.get_all<Subscriber>(where(c(&Subscriber::tmsi) == tmsi));
    if (results.empty()) return std::nullopt;
    return results.front();
}

void XLR::updateTmsi(const std::string &imsi, const std::string &tmsi) {
    std::lock_guard lock(mtx);
    db.update_all(
       set(c(&Subscriber::tmsi) = tmsi),
       where(c(&Subscriber::imsi) == imsi)
   );
}

void XLR::updateEnodeB(const std::string& tmsi, int enodebId) {
    std::lock_guard lock(mtx);
    db.update_all(
        set(c(&Subscriber::enodeb_id) = enodebId),
        where(c(&Subscriber::tmsi) == tmsi)
    );
}

void XLR::clearTmsi(const std::string& tmsi) {
    std::lock_guard lock(mtx);
    db.update_all(
        set(c(&Subscriber::tmsi) = std::string(""), c(&Subscriber::enodeb_id) = -1),
        where(c(&Subscriber::tmsi) == tmsi)
    );
}

std::optional<Subscriber> XLR::findByMsisdn(const std::string &msisdn) {
    std::lock_guard lock(mtx);
    auto results = db.get_all<Subscriber>(where(c(&Subscriber::msisdn) == msisdn));
    if (results.empty()) return std::nullopt;
    return results.front();
}

