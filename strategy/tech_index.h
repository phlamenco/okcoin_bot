//
// Created by liaosiwei on 17/1/15.
//

#ifndef OKCOIN_BOT_TECH_INDEX_H
#define OKCOIN_BOT_TECH_INDEX_H

#include <vector>

inline double ROC(double today, double n_days_ago) {
    return today * 100 / n_days_ago - 100;
}

inline double MAROC(const std::vector<std::pair<int64_t , double>>& roc_vec) {
    double sum = 0;
    for (auto roc : roc_vec)
        sum += roc.second;
    return sum / roc_vec.size();
}

template <typename T>
T SUM(const std::vector<T>& vec) {
    T sum = 0;
    for(auto i : vec)
        sum += i;
    return sum;
}

template <typename T>
T AVERAGE(const std::vector<T>& vec) {
    return SUM(vec) / vec.size();
}

template <typename T>
T MIN(const std::vector<T>& vec) {
    if (vec.size() == 1)
        return vec[0];
    else if (vec.size() > 1) {
        auto min = vec[0];
        for (auto i = 1; i < vec.size(); i++) {
            if (min > vec[i])
                min = vec[i];
        }
        return min;
    }
    return -1;
}

#endif //OKCOIN_BOT_TECH_INDEX_H
