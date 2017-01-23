//
// Created by liaosiwei on 17/1/7.
//

#ifndef OKCOIN_BOT_SINGLETON_H
#define OKCOIN_BOT_SINGLETON_H

template <typename T>
class Singleton {
public:
    static T& instance() {
        static T inst;
        return inst;
    }
    Singleton(Singleton const&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton const&) = delete;
    Singleton& operator=(Singleton &&) = delete;
};

#endif //OKCOIN_BOT_SINGLETON_H
