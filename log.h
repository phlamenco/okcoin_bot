//
// Created by liaosiwei on 17/1/8.
//

#ifndef OKCOIN_BOT_LOG_H
#define OKCOIN_BOT_LOG_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <iostream>

class Log {
public:
    int init(const std::string& log_file) {
        try {
             //queue size must be power of 2
            //spdlog::set_async_mode(q_size);
            auto file_logger = spdlog::rotating_logger_mt(logger_name, log_file, 1024 * 1024 * 5, 3);
            file_logger->set_pattern("%l %Y-%m-%d %H:%M:%S %t * %v");
            file_logger->set_level(spdlog::level::debug);
        } catch (const spdlog::spdlog_ex& ex) {
            std::cout << "Log initialization failed: " << ex.what() << std::endl;
            return -1;
        }
        return 0;
    }

private:
    // when queue is full, async will be turned into sync
    //size_t q_size = 8192;
    const std::string logger_name = "okbot_logger";
};

#define OKBOT_LOG_NOTICE(_fmt_, args...) do {\
spdlog::get("okbot_logger")->info(_fmt_, ##args); \
spdlog::get("okbot_logger")->flush(); \
} while(0)\

#define OKBOT_LOG_FATAL(_fmt_, args...) do {\
spdlog::get("okbot_logger")->error(_fmt_, ##args); \
spdlog::get("okbot_logger")->flush();\
} while(0)\

#define OKBOT_LOG_WARNING(_fmt_, args...) do {\
spdlog::get("okbot_logger")->warn(_fmt_, ##args); \
spdlog::get("okbot_logger")->flush();\
} while(0)\

#define OKBOT_LOG_DEBUG(_fmt_, args...) do {\
spdlog::get("okbot_logger")->debug(_fmt_, ##args); \
spdlog::get("okbot_logger")->flush();\
} while(0)\

#endif //OKCOIN_BOT_LOG_H
