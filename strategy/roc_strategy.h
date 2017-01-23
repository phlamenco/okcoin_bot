//
// Created by liaosiwei on 17/1/17.
//

#ifndef OKCOIN_BOT_ROC_STRATEGY_H
#define OKCOIN_BOT_ROC_STRATEGY_H

#include <rapidjson/document.h>
#include <string>
#include "util/time_util.h"
#include <util/time_window.h>
#include <sstream>
#include <thread>
#include <pipeline.h>
#include <util/singleton.h>
#include <util/window.h>
#include "tech_index.h"
#include "util/json_util.h"
#include "log.h"
#include "dump.h"
#include "account_status.h"
#include "asset_freeze.h"

#define COUNT 12
#define AVG_NUM 6

class RocStrategy {
public:

    void set_asset_and_security(double s, double sec) {
        asset = s;
        security = sec;
    }

    void set_interval(int milliseconds) {
        freq = milliseconds;
    }

    void run() {
        t1_ = std::thread(&RocStrategy::run_strategy, this);
        t2_ = std::thread(&RocStrategy::risk_control, this);
        t3_ = std::thread(&RocStrategy::query_asset, this);
    }

    void query_asset() {
        auto& trade = Singleton<Trade>::instance();
        while (inner_sig != 1) {
            //if (st.get_status() == Status::in_trading) {
            if (trade.ok_spotcny_userinfo() == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(freq));
            } else {
                OKBOT_LOG_FATAL("{}", "send user info request failed. resend after 400ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
            }
            //}
        }
    }

    void risk_control() {
        auto& trade = Singleton<Trade>::instance();
        while (signal != 1) {
            auto& user_info_queue = Singleton<Pipeline>::instance().get_queue(
                    "ok_spotcny_userinfo");
            auto& st = Singleton<AccountStatus>::instance();
            std::shared_ptr<rapidjson::Document> user_info_data;
            user_info_data = user_info_queue.pop();
            auto data_node = json_util::get_data(user_info_data.get(),
                                                 "ok_spotcny_userinfo");
            if (data_node != nullptr) {
                if (st.get_status() == Status::in_trading) {
                    double total, cny_free;
                    if (json_util::get_value(data_node, "info.funds.asset.total", total) == 0 &&
                        json_util::get_value(data_node, "info.funds.free.cny", cny_free) == 0) {
                        auto current_asset = (total - cny_free) / (trading_btc + origin_asset) * trading_btc.load();
                        auto profit = (current_asset - trading_asset.load()) / trading_asset.load() * 100;
                        OKBOT_LOG_NOTICE("current_asset={} trade_asset={} profit={}%", current_asset, trading_asset.load(), profit);
                        profit_win.put(profit);
                        if (profit_win.is_full()) {
                            security = MIN(profit_win.get_range(0, profit_win.get_size()));
                            if (profit < security) {
                                //触发止损策略
                                std::lock_guard<std::mutex> lock(mutex_);
                                int retry = 1;
                                while (retry < 500 && trading_btc.load() > 0) {
                                    if (trade.sell_at_market(trading_btc) == 0) {
                                        OKBOT_LOG_WARNING(
                                                "trigger risk control, sell all bitcoin with total asset={} profit={} security={}",
                                                current_asset, profit, security);
                                        OKBOT_LOG_NOTICE("op={} asset={} btc={}", "sell", trading_asset.load(),
                                                         trading_btc.load());
                                        trading_btc = 0;
                                        trading_asset = 0;
                                        profit_win.clear();
                                        st.set_status(Status::out_of_trading);
                                        break;
                                    } else {
                                        retry++;
                                        // 需要尽快抛售
                                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                                        OKBOT_LOG_WARNING(
                                                "sell btc by risk control failed, after sleep 2ms retry: {} ",
                                                retry);
                                    }
                                }
                            }
                        }

                    } else {
                        OKBOT_LOG_FATAL("can't get total and free cny: {}",
                                        json_util::json_to_str(user_info_data.get(), user_info_data->GetAllocator()));
                    }
                }
            }
/*           } else {
//                ("invalid mesg format {}",
//                                json_util::json_to_str(user_info_data.get(), user_info_data->GetAllocator()));
            }*/
        }
        inner_sig++;
    }

    void run_strategy() {
        auto& queue = Singleton<Pipeline>::instance().get_queue(strategy_data_source);
        auto& st = Singleton<AccountStatus>::instance();
        auto& trade = Singleton<Trade>::instance();
        while (signal == 0) {
            std::shared_ptr<rapidjson::Document> node;
            node = queue.pop();
            auto updated = calc_roc_and_maroc(node);
            // 保护策略，如果发生突发性抛售，也提前卖出
//            if (st.get_status() == Status::in_trading && sudden_down()) {
//                OKBOT_LOG_WARNING("{}", "trigger protect strategy, sell all btcoin.");
//                std::lock_guard<std::mutex> lock(mutex_);
//                if (trading_btc.load() > 0 && trade.sell_at_market(trading_btc) == 0) {
//                    OKBOT_LOG_NOTICE("op={} asset={} btc={}", "sell", trading_asset.load(), trading_btc.load());
//                    st.set_status(Status::out_of_trading);
//                    trading_asset = 0;
//                    trading_btc = 0;
//                }
//            }
            if (updated) {
                if (st.get_status() == Status::in_trading) {
                    if (cross_for_down()) {
                        std::lock_guard<std::mutex> lock(mutex_);
                        if (trading_btc.load() > 0 && trade.sell_at_market(trading_btc) == 0) {
                            OKBOT_LOG_NOTICE("op={} asset={} btc={}", "sell", trading_asset.load(), trading_btc.load());
                            st.set_status(Status::out_of_trading);
                            trading_asset = 0;
                            trading_btc = 0;
                            profit_win.clear();
                        }
                   }
                } else if (st.get_status() == Status::out_of_trading) {
                    if (cross_for_up() || stay_for_up()) {
                        auto ret = trade.ok_spotcny_trade("btc_cny",
                                                    "buy_market",
                                                    std::to_string(asset),
                                                    ""
                        );
                        if (ret == 0) {
                            auto& trade_queue = Singleton<Pipeline>::instance().get_queue("ok_spotcny_trade");
                            std::shared_ptr<rapidjson::Document> data;
                            trade_queue.pop(data, 10000);
                            if (data && data->IsArray()) {
                                for (auto size = 0; size < data->Size(); size++) {
                                    if (std::string("ok_spotcny_trade") == (*data)[size]["channel"].GetString()) {
                                        std::string res;
                                        json_util::get_value(&(*data)[size], "data.result", res);
                                        if (res == "true") {
                                            std::string order_id;
                                            if (json_util::get_value(&(*data)[size], "data.order_id", order_id) == 0) {
                                                auto& order_queue = Singleton<Pipeline>::instance().get_queue(
                                                        "ok_spotcny_orderinfo");
                                                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                                                if (trade.ok_spotcny_orderinfo(order_id) == 0) {
                                                    std::shared_ptr<rapidjson::Document> order_info;
                                                    order_queue.pop(order_info, 10000);
                                                    if (order_info && order_info->IsArray() && order_info->Size() > 0) {
                                                        int status;
                                                        if (json_util::get_value(&(*order_info)[0],
                                                                                 "data.orders\\0.status", status) == 0) {
                                                            if (status == 2 || status == -1) {
                                                                double avg_price = 0;
                                                                double deal_amount = 0;
                                                                auto ret1 = json_util::get_value(&(*order_info)[0],
                                                                                                 "data.orders\\0.avg_price",
                                                                                                 avg_price);
                                                                auto ret2 = json_util::get_value(&(*order_info)[0],
                                                                                                 "data.orders\\0.deal_amount",
                                                                                                 deal_amount);
                                                                if (ret1 == 0 && ret2 == 0) {
                                                                    if (status == 2) {
                                                                        trading_asset = avg_price * deal_amount;
                                                                        trading_btc = deal_amount;
                                                                        st.set_status(Status::in_trading);
                                                                    } else {
                                                                        origin_asset.store(origin_asset.load() + deal_amount);
                                                                        OKBOT_LOG_NOTICE("order is partially checked with deal_amount={}",
                                                                                         deal_amount);
                                                                    }
                                                                    OKBOT_LOG_NOTICE("op={} asset={} btc={}", "buy",
                                                                                     trading_asset.load(),
                                                                                     trading_btc.load()
                                                                    );
                                                                } else {
                                                                    OKBOT_LOG_FATAL("{}", "failed to get order info.");
                                                                }
                                                                break;
                                                            } else {
                                                                OKBOT_LOG_FATAL("invalid status for market buy. status={}", status);
                                                            }
                                                        }
                                                    }

                                                }
                                            }

                                        } else
                                            OKBOT_LOG_FATAL("{} with mesg={}",
                                                            "buy failed",
                                                            json_util::json_to_str(data.get(),
                                                                                   data->GetAllocator()));
                                    }
                                }
                            } else {
                                OKBOT_LOG_FATAL("get data from queue failed: {}", "");
                            }

                        } else {
                            OKBOT_LOG_FATAL("{}", "buy in roc_strategy failed.");
                        }
                    }
                } else {
                    OKBOT_LOG_FATAL("invalid status for trade {}", int(st.get_status()));

                }
            }
        }
        // when quit, we sell all our bitcoin
        if (trading_btc.load() > 0) {
            OKBOT_LOG_NOTICE("start sell all bitcoin={} in roc strategy.", trading_btc.load());
            trade.sell_at_market(trading_btc);
            OKBOT_LOG_NOTICE("{}", "end sell all bitcoin roc strategy.");
        }
    }

    void stop() {
        signal++;
    }

    void join() {
        OKBOT_LOG_NOTICE("{}", "start join roc strategy.");
        t1_.join();
        t2_.join();
        t3_.join();
        OKBOT_LOG_NOTICE("{}", "joined roc strategy.");
    }

    // 计算ROC(12, 6) 周期1min, 如果maroc过去一分钟的数据更新了就返回0，否者-1
    bool calc_roc_and_maroc(std::shared_ptr<rapidjson::Document> node) {
        std::ostringstream ss;
        bool updated = false;
        if (node->IsArray()) {
            for (auto k = 0; k < node->Size(); k++) {
                if (strategy_data_source != (*node)[k]["channel"].GetString())
                    continue;
                auto new_node = json_util::get_node(&((*node)[k]), "data");
                if (new_node != nullptr) {
                    if (new_node->IsArray() && new_node->Size() > 0) {
                        if ((*new_node)[0].IsArray()) {
                            for (auto i = 0; i < new_node->Size(); i++) {
                                auto &data_node = (*new_node)[i];
                                ss << time2str(data_node[0].GetInt64())
                                   << "\t" << data_node[4].GetDouble();
                                if (i != (new_node->Size() - 1))
                                    ss << "\n";

                                auto newest_timer = data_node[0].GetInt64();
                                window.put(newest_timer, data_node[4].GetDouble());
                                if (window.is_full()) {
                                    //计算1min的ROC(12, 6)
                                    roc_win.put(newest_timer,
                                                ROC(window.get(0).second, window.get(COUNT - 1).second));
                                    if (roc_win.get_size() >= AVG_NUM) {
                                        updated = (newest_timer != maroc_win[0].first);
                                        if (maroc_win.put(newest_timer, MAROC(roc_win.get_range(0, AVG_NUM))) == 0 &&
                                            maroc_win.get_size() > 1) {

                                            DUMP_DATA("roc", "{}\t{}\t{}\t{}", time2str(window.get(1).first),
                                                      window.get(1).second, roc_win.get(1).second,
                                                      maroc_win.get(1).second);
                                        }
                                    }
                                    //DUMP_DATA("debug", "[window]\n{}\n[roc]\n{}\n[maroc]\n{}", roc_win.debug_str(), maroc_win.debug_str());
                                }
                            }
                        } else {
                            ss << time2str((*new_node)[0].GetInt64())
                               << "\t" << (*new_node)[4].GetDouble();

                            auto newest_timer = (*new_node)[0].GetInt64();
                            window.put(newest_timer, (*new_node)[4].GetDouble());
                            if (window.is_full()) {
                                roc_win.put(newest_timer, ROC(window.get(0).second, window.get(COUNT - 1).second));
                                if (roc_win.get_size() >= AVG_NUM) {
                                    updated = (newest_timer != maroc_win[0].first);
                                    if (maroc_win.put(newest_timer, MAROC(roc_win.get_range(0, AVG_NUM))) == 0 &&
                                        maroc_win.get_size() > 1) {
                                        DUMP_DATA("roc", "{}\t{}\t{}\t{}", time2str(window.get(1).first),
                                                  window.get(1).second, roc_win.get(1).second,
                                                  maroc_win.get(1).second);
                                    }
                                }
                                //DUMP_DATA("debug", "[window]\n{}\n[roc]\n{}\n[maroc]\n{}", window.debug_str(), roc_win.debug_str(), maroc_win.debug_str());
                            }
                        }
                        DUMP_DATA("kline", "{}", ss.str());
                        ss.str("");
                    }
                }
            }
        }
        return updated;
    }

    bool calc_s_roc(std::shared_ptr<rapidjson::Document> doc) {
        bool updated = false;
        auto node = json_util::get_data(doc.get(), strategy_data_source);
        if (node != nullptr) {
            // TODO
        } else {
            OKBOT_LOG_FATAL("invalid format for calc_s_roc: {}",
                            json_util::json_to_str(doc.get(), doc->GetAllocator()));
        }
        return updated;
    }

private:
    bool cross_for_up() {
        if (maroc_win.get_size() >= 3) {
            if (roc_win[1].second > maroc_win[1].second &&
                roc_win[2].second < maroc_win[2].second &&
                maroc_win[1].second > maroc_win[2].second)
                return true;
        }
        return false;
    }
    bool stay_for_up() {
        if (maroc_win.get_size() >= 3) {
            if (roc_win[1].second > maroc_win[1].second &&
                roc_win[2].second > maroc_win[2].second &&
                maroc_win[1].second > maroc_win[2].second)
                return true;
        }
        return false;
    }
    bool cross_for_down() {
        if (maroc_win.get_size() >= 3) {
            if (roc_win[1].second < maroc_win[1].second &&
                roc_win[2].second > maroc_win[2].second)
                return true;
        }
        return false;
    }

//    bool sudden_down() {
//        bool res = false;
//        if (window.get_size() >= COUNT) {
//            double diff = 0;
//            for (auto i = 1; i < COUNT; i++) {
//                diff += window[i].second - window[i-1].second;
//            }
//            res = (diff / (COUNT-1)) > 5;
//        }
//        return res;
//    }

    const std::string strategy_data_source = "ok_sub_spotcny_btc_kline_1min";
    TimeWindow<double, COUNT> window;
    TimeWindow<double, 720> roc_win;
    TimeWindow<double, 720> maroc_win;
    Window<double, 12> profit_win;
    std::atomic<double> trading_asset {0};
    std::atomic<double> trading_btc {0};
    std::atomic<double> origin_asset {0.0004};
    std::thread t1_;
    std::thread t2_;
    std::thread t3_;
    std::atomic<int> signal{0};
    std::atomic<int> inner_sig{0};
    int freq;
    double security = 0;
    double asset = 0;
    std::mutex mutex_;
};

#endif //OKCOIN_BOT_ROC_STRATEGY_H
