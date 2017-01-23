//
// Created by liaosiwei on 17/1/7.
//

#ifndef OKCOIN_BOT_CONFIGURE_H
#define OKCOIN_BOT_CONFIGURE_H

#include "cpptoml.h"
#include <fstream>

class Configure {
public:
    int init(const std::string& file) {
        try {
            config = cpptoml::parse_file(file);
        } catch (...) {
            return -1;
        }
        return 0;
    }
private:
    std::shared_ptr<cpptoml::table> config;
};

#endif //OKCOIN_BOT_CONFIGURE_H
