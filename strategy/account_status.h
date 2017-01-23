//
// Created by liaosiwei on 17/1/18.
//

#ifndef OKCOIN_BOT_ACCOUNT_STATUS_H
#define OKCOIN_BOT_ACCOUNT_STATUS_H

#include <mutex>

enum class Status {
    in_trading,
    out_of_trading
};

class AccountStatus {
public:
    Status get_status() {
        std::lock_guard<std::mutex> lock(mutex_);
        return status;
    }

    void set_status(Status status) {
        std::lock_guard<std::mutex> lock(mutex_);
        AccountStatus::status = status;
    }

private:
    std::mutex mutex_;
    Status status = Status::out_of_trading;
};

#endif //OKCOIN_BOT_ACCOUNT_STATUS_H
