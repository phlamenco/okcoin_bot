//
// Created by liaosiwei on 17/1/14.
//

#ifndef OKCOIN_BOT_ASSET_FREEZE_H
#define OKCOIN_BOT_ASSET_FREEZE_H

#include <util/singleton.h>
#include <trade.h>
#include "numeric_util.h"
#include "account_status.h"

//TODO use adjustable standard_asset

class AssetFreeze {
public:

    AssetFreeze() {

    }

    void set_global_security(double asset, double l) {
        standard_asset = asset;
        level = l;
    }

    void set_freq(int f) {
        freq = f;
    }

    bool is_worked() {
        return worked.load();
    }

    void reset() {
        worked.store(false);
    }

    double current_btc() {
        return btc.load();
    }

    double current_trading_asset() {
        return trading_asset.load();
    }

    void run() {
        t1_ = std::thread(&AssetFreeze::global_asset_freeze, this);
        t2_ = std::thread(&AssetFreeze::query_asset, this);
    }

    void stop() {
        signal++;
    }

    void join() {
        OKBOT_LOG_NOTICE("{}", "start join asset freeze.");
        t1_.join();
        t2_.join();
        OKBOT_LOG_NOTICE("{}", "joined asset freeze.");
    }

    void query_asset() {
        auto& trade = Singleton<Trade>::instance();
        auto& st = Singleton<AccountStatus>::instance();
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

    void global_asset_freeze() {
        auto& trade = Singleton<Trade>::instance();
        while (signal != 1) {
            auto& queue = Singleton<Pipeline>::instance().get_queue("ok_spotcny_userinfo");
            auto& st = Singleton<AccountStatus>::instance();
            std::shared_ptr<rapidjson::Document> data;
            data = queue.pop();
            if (data) {
                double total = 0;
                double res = 0;
                double btc_free = 0;
                double cny_free = 0;
                auto data_node = json_util::get_data(data.get(), "ok_spotcny_userinfo");
                if (data_node != nullptr) {
                    if (json_util::get_value(data_node, "info.funds.free.btc", btc_free) == 0 &&
                        json_util::get_value(data_node, "info.funds.asset.total", total) == 0 &&
                        json_util::get_value(data_node, "info.funds.free.cny", cny_free) == 0) {
                        // 本策略的思路是 如果当前账户总资产减少到小于之前定下的总资产的一定百分比，那么强制退场。
                        res = total - cny_free;
                        // 记录下当前bitcoin个数以及对应的人民币数额
                        trading_asset = res;
                        btc = btc_free;

                        auto target_asset = standard_asset * (1 + level);
                        if (st.get_status() == Status::in_trading &&
                            res < target_asset && btc_free >= 0.01) {
                            // sell all bitcoin
                            int retry = 1;
                            while (retry < 500) {
                                if (trade.sell_at_market(btc_free) == 0) {
                                    st.set_status(Status::out_of_trading);
                                    worked = true;
                                    OKBOT_LOG_WARNING(
                                            "trigger global asset freeze, sell all bitcoin with total asset {}",
                                            res);
                                    trade.close_trade_ugly();
                                    break;
                                }
                                else {
                                    retry++;
                                    // 需要尽快抛售
                                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                                    OKBOT_LOG_WARNING(
                                            "sell btc by strategy AssetFreeze failed, after sleep 2ms retry: {} ",
                                            retry);
                                }
                            }
                            auto profit = (res - standard_asset) / standard_asset;
                            OKBOT_LOG_NOTICE("trade_asset={} total_asset={} profit={}% quit_asset={}",
                                             standard_asset, res, double2str(profit*100, 2), target_asset);
                        }
                    } else {
                        OKBOT_LOG_FATAL("parse free.btc failed: {}",
                                        json_util::json_to_str(data.get(), data->GetAllocator()));
                    }
                } else {
                    OKBOT_LOG_FATAL("invalid mesg format {}",
                                    json_util::json_to_str(data.get(), data->GetAllocator()));
                }
            }
        }
        inner_sig++;
    }
private:
    std::atomic<double> standard_asset;
    std::atomic<double> level;
    std::atomic<double> trading_asset;
    std::atomic<int> freq;
    std::thread t1_;
    std::thread t2_;
    std::atomic<int> signal {0};
    std::atomic<int> inner_sig {0};
    std::atomic<bool> worked {false};
    std::atomic<double> btc {0.0};
};



#endif //OKCOIN_BOT_ASSET_FREEZE_H
