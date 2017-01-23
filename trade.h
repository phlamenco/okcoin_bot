//
// Created by liaosiwei on 17/1/9.
//

#ifndef OKCOIN_BOT_TRADE_H
#define OKCOIN_BOT_TRADE_H

#include "parameter.h"
#include <string>
#include <util/json_util.h>
#include "websocket.h"
#include "log.h"
#include "pipeline.h"
#include "singleton.h"

class Trade {
public:
    void init(WebSocket *s, const std::string& api_key, const std::string& secret_key) {
        s_ = s;
        m_api_key = api_key;
        m_secret_key = secret_key;
    }
    void set_websocket(WebSocket* s) {
        s_ = s;
    }

    void close_trade_ugly() {
        if (s_ != 0 && s_->is_valid) {
            s_->doclose();
            s_ = 0;
        }
    }
// 看起来，这里不能使用带有超时设置的pop，websocket层保证了请求不会丢失，但是超时时间很难确定
#define CHECK_AND_RETURN(err_msg) do {\
    auto& queue = Singleton<Pipeline>::instance().get_queue("ok_spotcny_trade");\
    std::shared_ptr<rapidjson::Document> data;\
    queue.pop(data, 10000);\
    if (data && data->IsArray()) {\
for (auto size = 0; size < data->Size(); size++) { \
if (std::string("ok_spotcny_trade") == (*data)[size]["channel"].GetString()) {\
std::string res;\
        json_util::get_value(&(*data)[size], "data.result", res);\
        if (res == "true")\
            return 0;\
        else\
            OKBOT_LOG_FATAL("{} with mesg={}", err_msg, json_util::json_to_str(data.get(), data->GetAllocator()));\
}\
} \
    } else {\
OKBOT_LOG_FATAL("get data from queue failed: {}", ""); \
}\
} while(0)\

    int buy(double price, double amount) {
        if (price > 0 && price < 1000000) {
            std::lock_guard<std::mutex> locker(lock);
            auto ret = ok_spotcny_trade("btc_cny",
                             "buy",
                             std::to_string(price),
                             std::to_string(amount)
            );
            if (ret == 0) {
                CHECK_AND_RETURN("buy failed.");
            } else {
                OKBOT_LOG_FATAL("{}", "websocket is invalid.");
            }
        } else {
            OKBOT_LOG_FATAL("{} price is {}", "buy price is illegal.", price);
        }
        return -1;
    }
    // BTC :最少买入0.01个BTC 的金额(金额>0.01*卖一价)
    int buy_at_market(double price) {
        if (price > 0) {
            std::lock_guard<std::mutex> locker(lock);
            auto ret = ok_spotcny_trade("btc_cny",
                             "buy_market",
                             std::to_string(price),
                             ""
            );
            if (ret == 0) {
                CHECK_AND_RETURN("buy at market failed.");
            } else {
                OKBOT_LOG_FATAL("{}", "websocket is invalid for buy at market.");
            }
        } else {
            OKBOT_LOG_FATAL("buy_at_market price is illegal {}", price);
        }
        return -1;
    }

    int sell(double price, double amount) {
        if (price > 0 && price < 1000000) {
            std::lock_guard<std::mutex> locker(lock);
            auto ret = ok_spotcny_trade("btc_cny",
                                    "sell",
                             std::to_string(price),
                             std::to_string(amount)
            );
            if (ret == 0) {
                CHECK_AND_RETURN("sell failed.");
            } else {
                OKBOT_LOG_FATAL("{}", "websocket is invalid for sell.");
            }
        } else {
            OKBOT_LOG_FATAL("{}", "sell price is illegal.");

        }
        return -1;
    }

    int sell_at_market(double amount) {
        if (amount > 0) {
            std::lock_guard<std::mutex> locker(lock);
            auto ret = ok_spotcny_trade("btc_cny",
            "sell_market",
                                        "",
                             std::to_string(amount)
            );
            if (ret == 0) {
                CHECK_AND_RETURN("sell at market failed.");
            } else {
                OKBOT_LOG_FATAL("{}", "websocket is invalid for sell at market.");
            }
        } else {
            OKBOT_LOG_FATAL("sell at market amount is illegal: {}", amount);
        }
        return -1;
    }

    int cancel(const std::string& order_id) {
        std::lock_guard<std::mutex> locker(lock);
        auto ret = ok_spotcny_cancel_order("btc_cny", order_id);
        if (ret == 0) {
            auto &queue = Singleton<Pipeline>::instance().get_queue("ok_spotcny_cancel_order");
            std::shared_ptr<rapidjson::Document> data;
            data = queue.pop();
            if (data && data->IsArray()) {
                for (auto size = 0; size < data->Size(); size++) {
                    if (std::string("ok_spotcny_cancel_order") == (*data)[size]["channel"].GetString()) {
                        std::string res;
                        json_util::get_value(&(*data)[size], "data.result", res);
                        if (res == "true")
                            return 0;
                        else
                            OKBOT_LOG_FATAL("mesg={} with data.result={}", "cancel failed", res);
                    }
                }
            }
        } else {
            OKBOT_LOG_FATAL("{}", "websocket is invalid for cancel.");
        }
        return -1;
    }

    int ok_spotcny_trade(const std::string &symbol,
                          const std::string &type,
                          const std::string &price,
                          const std::string &amount) {
        Parameter prmt;
        prmt.AddParam("api_key",m_api_key);
        prmt.AddParam("symbol",symbol);
        prmt.AddParam("type",type);
        if (!price.empty())
            prmt.AddParam("price",price);
        if (!amount.empty())
            prmt.AddParam("amount",amount);

        std::string sign = prmt.GetSign(m_secret_key);
        prmt.AddParam("sign",sign);
        std::string prmtstr = prmt.ToJsonString();
        try {
            if (s_ != 0 && s_->is_valid)
                s_->emit("ok_spotcny_trade",prmtstr);
            return 0;
        } catch (...) {
            OKBOT_LOG_FATAL("{}", "ok_spotcny_trade failed.");
            return -1;
        }
    }

    int ok_spotcny_orderinfo(const std::string& order_id) {
        Parameter prmt;
        prmt.AddParam("api_key",m_api_key);
        prmt.AddParam("symbol","btc_cny");
        prmt.AddParam("order_id",order_id);
        string sign = prmt.GetSign(m_secret_key);
        prmt.AddParam("sign",sign);
        string prmtstr = prmt.ToJsonString();
        try {
            if (s_ != 0 && s_->is_valid)
                s_->emit("ok_spotcny_orderinfo",prmtstr);
            return 0;
        } catch (...) {
            OKBOT_LOG_FATAL("{}", "ok_spotcny_orderinfo failed.");
            return -1;
        }
    }

    int ok_spotcny_cancel_order(const std::string &symbol,const std::string &order_id) //取消订单
    {
        Parameter prmt;
        prmt.AddParam("api_key",m_api_key);
        prmt.AddParam("symbol",symbol);
        prmt.AddParam("order_id",order_id);
        string sign = prmt.GetSign(m_secret_key);
        prmt.AddParam("sign",sign);
        string prmtstr = prmt.ToJsonString();
        try {
            if (s_ != 0 && s_->is_valid)
                s_->emit("ok_spotcny_cancel_order",prmtstr);
            return 0;
        } catch (...) {
            OKBOT_LOG_FATAL("{}", "ok_spotcny_cancel_order failed.");
            return -1;
        }
    }

    int ok_spotcny_userinfo()
    {
        Parameter prmt;
        prmt.AddParam("api_key", m_api_key);
        string sign = prmt.GetSign(m_secret_key);
        prmt.AddParam("sign", sign);
        string prmtstr = prmt.ToJsonString();
        try {
            if (s_ != 0 && s_->is_valid)
                s_ ->emit("ok_spotcny_userinfo", prmtstr);
            return 0;
        } catch (...) {
            OKBOT_LOG_FATAL("{}", "ok_spotcny_userinfo failed");
            return -1;
        }
    }

    std::shared_ptr<rapidjson::Document> get_res_by_channel(const std::string& channel, int time_out = 2000) {
        auto& queue = Singleton<Pipeline>::instance().get_queue(channel);
        auto s = queue.size();
        if (s > 1) {
            OKBOT_LOG_FATAL("illegal queue size {}", s);
        }
        std::shared_ptr<rapidjson::Document> data;
        // set timeout to 1min
        queue.pop(data, time_out);
        return data;
    }

private:
    std::mutex lock;
    WebSocket* s_ = 0;
    std::string m_api_key;			//用户申请的apiKey
    std::string m_secret_key;		//请求参数签名的私钥
};

#endif //OKCOIN_BOT_TRADE_H
