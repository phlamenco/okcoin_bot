//
// Created by liaosiwei on 17/1/12.
//

#ifndef OKCOIN_BOT_TIMER_H
#define OKCOIN_BOT_TIMER_H

#include <chrono>

// non-thread-safe
template <typename TimeUnit>
class Timer {
public:

    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::steady_clock::time_point;

    Timer() : count_(0), flag_(0) {}
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void Set() {
        start_ = Clock::now();
        isSet = true;
    }

    // may overflow
    long Check() {
        if (!isSet)
            return -1;
        flag_ = 1;
        count_ =  (long)std::chrono::duration_cast<TimeUnit>(Clock::now() -
                                                             start_).count();
        return count_;
    }

    long Get() {
        if (!isSet)
            return -1;
        if (flag_ == 0) {
            return Check();
        }
        return count_;
    }

    void Clear() {
        count_ = 0;
        flag_ = 0;
        isSet = false;
    }

private:
    TimePoint start_;
    long count_;
    int flag_;
    bool isSet = false;
};

#endif //OKCOIN_BOT_TIMER_H
