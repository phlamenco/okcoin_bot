//
// Created by liaosiwei on 17/1/22.
//

#ifndef OKCOIN_BOT_WINDOW_H
#define OKCOIN_BOT_WINDOW_H

#include <array>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include "time_util.h"

template <typename T, size_t N>
class Window {
public:
    Window() {}

    // return value: 0 add, 1 rewrite, -1 do nothing
    void put(T elem) {
        std::lock_guard<std::mutex> locker(lock);

        window[index++] = elem;
        if (!full) {
            valid_size++;
            if (index == N) {
                full = true;
            }
        }
        index %= N;
        first = false;
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
            window[(index + N - 1 - place) % N] = elem;
        }
    }

    T get(size_t at) {
        if (at >= N || at < 0) {
            throw std::invalid_argument("invalid argument for TimeWindow::get");
        }
        std::lock_guard<std::mutex> locker(lock);
        return window[(index + N - 1 - at) % N];
    }

    T operator[] (size_t at) {
        return get(at);
    };

    std::vector<T> get_range(size_t beg, size_t length) {
        if (beg >= N || beg < 0 || length > N || length < 0) {
            throw std::invalid_argument("invalid argument for TimeWindow::get_range");
        }
        std::vector<T> res;
        res.reserve(N);
        std::lock_guard<std::mutex> locker(lock);
        for (auto i = 0; i < length; i++) {
            res.push_back(window[i]);
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
                ss << window[(index + N - 1 - i) % N] << "\t";
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
    std::array<T, N> window;
};


#endif //OKCOIN_BOT_WINDOW_H
