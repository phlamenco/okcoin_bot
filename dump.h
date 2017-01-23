//
// Created by liaosiwei on 17/1/11.
//

#ifndef OKCOIN_BOT_DUMP_H
#define OKCOIN_BOT_DUMP_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <iostream>

/*#define Q_SIZE 8192*/
/*spdlog::set_async_mode(Q_SIZE); \*/

#define REGISTER_DUMP(log_file) \
do {\
try {\
auto data_dump = spdlog::basic_logger_mt(log_file, log_file, false);\
data_dump->set_pattern("%v");\
}\
catch (const spdlog::spdlog_ex& ex) { \
std::cout << "Log initialization failed: " << ex.what() << std::endl; \
} \
} while(0)

#define DUMP_DATA(log_file, _fmt_, args...) do {\
spdlog::get(log_file)->info(_fmt_, ##args); \
spdlog::get(log_file)->flush();\
} while(0)

#endif //OKCOIN_BOT_DUMP_H
