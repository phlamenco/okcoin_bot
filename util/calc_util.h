//
// Created by liaosiwei on 17/1/11.
//

#ifndef OKCOIN_BOT_CALC_UTIL_H_H
#define OKCOIN_BOT_CALC_UTIL_H_H

#include <rapidjson/document.h>
#include <string>
#include <sstream>
#include <dump.h>
#include <strategy/tech_index.h>
#include "time_window.h"
#include "time_util.h"

inline void dump_trades_data(rapidjson::Value* node) {
    std::ostringstream ss;
    if (node->IsArray()) {
        for (auto i = 0; i < node->Size(); i++) {
            const auto& target_node = (*node)[i];
            if (target_node.HasMember("channel") &&
                std::string("ok_sub_spotcny_btc_trades") == target_node["channel"].GetString() &&
                target_node.HasMember("data")) {
                const auto& data = target_node["data"];
                if (data.IsArray()) {
                    for (auto j = 0; j < data.Size(); j++) {
                        const auto& elems = data[j];
                        if (elems.IsArray()) {
                            for (auto k = 0; k < elems.Size(); k++) {
                                ss << elems[k].GetString();
                                if (k != elems.Size() - 1)
                                    ss << "\t";
                            }
                            DUMP_DATA("trades_data", "{}", ss.str());
                            ss.str("");
                        }
                    }
                }
            }
        }
    }
}

inline void dump_depth_20_data(rapidjson::Value* node) {
    std::ostringstream ss;
    if (node->IsArray()) {
        for (auto i = 0; i < node->Size(); i++) {
            const auto& target_node = (*node)[i];
            if (target_node.HasMember("channel") &&
                std::string("ok_sub_spotcny_btc_depth_20") == target_node["channel"].GetString() &&
                target_node.HasMember("data")) {
                const auto& data = target_node["data"];
                std::string time_str;
                if (data.HasMember("timestamp")) {
                    time_str = time2str(data["timestamp"].GetString());
                }
                if (data.HasMember("bids")) {
                    const auto& bids = data["bids"];
                    for (auto j = 0; j < bids.Size(); j++) {
                        if (bids[j].Size() != 2)
                            OKBOT_LOG_FATAL("{}", "invalid format of depth_20 bids data");
                        ss << bids[j][0].GetDouble() << "\t" << bids[j][1].GetDouble() << "\t" << time_str << "\t" << "bid";
                        DUMP_DATA("depth_20", "{}", ss.str());
                        ss.str("");
                    }
                }
                if (data.HasMember("asks")) {
                    const auto& bids = data["asks"];
                    for (auto j = 0; j < bids.Size(); j++) {
                        if (bids[j].Size() != 2)
                            OKBOT_LOG_FATAL("{}", "invalid format of depth_20 asks data");
                        ss << bids[j][0].GetDouble() << "\t" << bids[j][1].GetDouble() << "\t" << time_str << "\t" << "ask";
                        DUMP_DATA("depth_20", "{}", ss.str());
                        ss.str("");
                    }
                }
            }
        }
    }
}

inline void dump_ticker(rapidjson::Value* node) {
    std::ostringstream ss;
    if (node->IsArray()) {
        for (auto i = 0; i < node->Size(); i++) {
            const auto& target_node = (*node)[i];
            if (target_node.HasMember("channel") &&
                std::string("ok_sub_spotcny_btc_ticker") == target_node["channel"].GetString() &&
                target_node.HasMember("data")) {
                const auto& data = target_node["data"];
                ss << data["last"].GetString()
                   << "\t" << data["buy"].GetString()
                   << "\t" << data["sell"].GetString()
                   << "\t" << time2str(data["timestamp"].GetString())
                   << "\t" << data["vol"].GetString();
                DUMP_DATA("ticker", "{}", ss.str());
                ss.str("");
            }
        }
    }
}

#endif //OKCOIN_BOT_CALC_UTIL_H_H
