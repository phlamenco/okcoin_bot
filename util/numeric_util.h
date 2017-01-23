//
// Created by liaosiwei on 17/1/15.
//

#ifndef OKCOIN_BOT_NUMERIC_UTIL_H
#define OKCOIN_BOT_NUMERIC_UTIL_H

#include <string>

// precision表示保留多少位小数
inline std::string double2str(double a, int precision = 6) {
    char str[32];
    auto ret = snprintf(str, sizeof(str), "%.*f", precision, a);
    if (ret >= 0)
        return std::string(str);
    else
        return std::string();
}

#endif //OKCOIN_BOT_NUMERIC_UTIL_H
