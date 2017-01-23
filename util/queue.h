//
// Created by liaosiwei on 17/1/14.
//

#ifndef OKCOIN_BOT_QUEUE_H
#define OKCOIN_BOT_QUEUE_H

#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

template <typename T>
class Queue
{
public:
    // pop with no timeout, may block forever
    T pop()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        cond_.wait(mlock, [this]{ return !queue_.empty();});
        auto item = queue_.front();
        queue_.pop();
        return item;
    }
    bool pop_nonblock(T& item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        if (!queue_.empty()) {
            item = queue_.front();
            queue_.pop();
            return true;
        }
        return false;
    }
    // pop with timeout of milliseconds, default timeout is 500ms
    bool pop(T& item, int time_out = 500)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto res = cond_.wait_until(mlock, now + std::chrono::milliseconds(time_out),
                                    [this]{return !queue_.empty();});
        if (res) {
            item = queue_.front();
            queue_.pop();
        }
        return res;
    }

    void push(const T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(item);
        mlock.unlock();
        cond_.notify_one();
    }

    void push(T&& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(std::move(item));
        mlock.unlock();
        cond_.notify_one();
    }

    size_t size() {
        std::unique_lock<std::mutex> mlock(mutex_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};


#endif //OKCOIN_BOT_QUEUE_H
