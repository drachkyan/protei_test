#pragma once

namespace StatusCode {
    constexpr int SUCCESS = 200;
    constexpr int NOT_FOUND = 404;
    constexpr int SERVER_ERROR = 500;

    const json SUCCESS_JSON = {{"status", SUCCESS}};
    const json NOT_FOUND_JSON = {{"status", NOT_FOUND}};
    const json SERVER_ERROR_JSON = {{"status", SERVER_ERROR}};
}

