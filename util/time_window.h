//
// Created by liaosiwei on 17/1/15.
//

#ifndef OKCOIN_BOT_TIME_WINDOW_H
#define OKCOIN_BOT_TIME_WINDOW_H

#include <array>
#include <mutex>
#include <vector>
#include "time_util.h"

template <typename T, size_t N>
class TimeWindow {
public:
    TimeWindow() {}

    // return value: 0 add, 1 rewrite, -1 do nothing
    int put(int64_t timestap, T elem) {
        std::lock_guard<std::mutex> locker(lock);
        if (first) {
            window[index++] = std::make_pair(timestap, elem);
            if (!full) {
                valid_size++;
                if (index == N) {
                    full = true;
                }
            }
            index %= N;
            first = false;
            return 0;
        } else {
            if (window[(index + N - 1) % N].first == timestap) {
                window[(index + N - 1) % N].second = elem;
                return 1;
            } else if (window[(index + N - 1) % N].first < timestap) {
                window[index++] = std::make_pair(timestap, elem);
                if (!full) {
                    valid_size++;
                    if (index == N)
                        full = true;
                }
                index %= N;
                return 0;
            }
        }
        return -1;
    }

    bool is_full() const {
        std::lock_guard<std::mutex> locker(lock);
        return full;
    }

    size_t get_size() const {
        std::lock_guard<std::mutex> locker(lock);
        return valid_size;
    }

    void update(T elem, size_t place) {
        if (place >= N || place < 0) {
            std::lock_guard<std::mutex> locker(lock);
            window[(index + N - 1 - place) % N].second = elem;
        }
    }

    std::pair<int64_t, T> get(size_t at) {
        if (at >= N || at < 0) {
            throw std::invalid_argument("invalid argument for TimeWindow::get");
        }
        std::lock_guard<std::mutex> locker(lock);
        return window[(index + N - 1 - at) % N];
    }

    std::pair<int64_t, T> operator[] (size_t at) {
        return get(at);
    };

    std::vector<std::pair<int64_t, T>> get_range(size_t beg, size_t length) {
        if (beg >= N || beg < 0 || length >= N || length < 0) {
            throw std::invalid_argument("invalid argument for TimeWindow::get_range");
        }
        if (beg + length > N)
            length = N - beg;

        std::vector<std::pair<int64_t, T>> res;
        res.reserve(N);
        std::lock_guard<std::mutex> locker(lock);
        for (auto i = 0; i < length; i++) {
            res.push_back(window[(beg - i + index + N - 1) % N]);
        }
        return res;
    }

    void clear() {
        std::lock_guard<std::mutex> locker(lock);
        full = false;
        valid_size = 0;
        first = true;
        index = 0;
    }

    std::string debug_str() {
        std::ostringstream ss;
        {
            std::lock_guard<std::mutex> locker(lock);
            for (auto i = 0; i < valid_size; i++) {
                ss << time2str(window[(index + N - 1 - i) % N].first) << "\t" << window[(index + N - 1 - i) % N].second
                   << "\n";
            }
        }
        return ss.str();
    }

private:
    std::mutex lock;
    // index indicate the latest element in the array
    bool full = false;
    size_t valid_size = 0;
    bool first = true;
    size_t index = 0;
    std::array<std::pair<int64_t, T>, N> window;
};

#endif //OKCOIN_BOT_TIME_WINDOW_H
