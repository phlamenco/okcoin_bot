//
// Created by liaosiwei on 17/1/15.
//

#ifndef OKCOIN_BOT_TIME_UTIL_H
#define OKCOIN_BOT_TIME_UTIL_H

#include <ctime>
#include <string>
#include <cstdint>

inline std::string time2str(const std::string& time_str) {
    try {
        long long time_val = std::stoll(time_str);
        std::time_t seconds = time_val / 1000;
        std::tm tm = *std::localtime(&seconds);
        char res[32];
        strftime(res, sizeof(res), "%H:%M:%S", &tm);
        return std::string(res);
    } catch (...) {
        return "";
    }
}

inline std::string time2str(int64_t time_val) {
    std::time_t seconds = time_val / 1000;
    std::tm tm = *std::localtime(&seconds);
    char res[32];
    strftime(res, sizeof(res), "%H:%M:%S", &tm);
    return std::string(res);
}

#endif //OKCOIN_BOT_TIME_UTIL_H
