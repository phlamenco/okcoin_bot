#include <util/singleton.h>
#include <strategy/asset_freeze.h>
#include <strategy/account_status.h>
#include <strategy/roc_strategy.h>
#include "okcoinwebsocketapi.h"
#include "rapidjson/document.h"
#include "log.h"
#include "trade.h"
#include "dump.h"
#include "util/calc_util.h"
#include "util/json_util.h"
#include "pipeline.h"

OKCoinWebSocketApiCn *cnapi = 0;	//此处为全局变量，建议用户做成单例模式

//以下是webstocket回调，由于国际站和国内站分别是两个连接，
//也就是数据是由两个线程发出的，故国际站和国内站有各自的回调函数。
//如果对线程应用不太熟练的开发者，尽量不要用一个回调函数同时接收两个站的数据，那样会使你的程序变得复杂。
void cn_callbak_open()
{
	//向服务器发送命令
	std::cout << "国内站连接成功！" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
	//连接成功后立即接收tick和depth数据
	//另外，把接收行情数据请求放在open回调里作用在于:
	//当意外断开，重新连接后触发本回调可自动发送接收请求。
	//所以尽量要把行情类的接收请求放在本回调里。
	if(cnapi != 0)
	{
		//cnapi->ok_spotcny_btc_depth_20();
        //cnapi->ok_spotcny_btc_trades();
        //cnapi->ok_spotcny_btc_ticker();
        //cnapi->ok_spotcny_btc_kline_5min();
        cnapi->ok_spotcny_btc_kline_1min();
	}
};
void cn_callbak_close()
{
	std::cout << "连接已经断开！ " << std::endl;
};
void cn_callbak_message(void* api, const char *message)
{
    if (api != nullptr) {
        // dispatch message to different queue
        using namespace rapidjson;
        const std::shared_ptr<rapidjson::Document> d(new rapidjson::Document());
        d->Parse(message);
        std::cout << json_util::json_to_str(d.get(), d->GetAllocator()) << std::endl;
        auto& pipeline = Singleton<Pipeline>::instance();
        if (d->IsArray()) {
            for (auto i = 0; i < d->Size(); i++) {
                auto& channel = (*d)[i];
                if (channel.HasMember("channel")) {
                    auto& queue = pipeline.get_queue(channel["channel"].GetString());
                    // 此处之所以没有使用shared_ptr<Value*>是因为它无法保证该指针的生命周期
                    queue.push(d);
                }
            }
        }
    } else {
        OKBOT_LOG_FATAL("{}", "dynamic dispatch failed in message callback.");
    }
};


int main(int argc, char* argv[]) 
{
    //初始化data dump,必须在log初始化之前，因为这里的数据打印是同步的，log是异步的
    REGISTER_DUMP("trades_data");
    REGISTER_DUMP("depth_20");
    REGISTER_DUMP("ticker");
    REGISTER_DUMP("kline");
    REGISTER_DUMP("roc");
    REGISTER_DUMP("debug");
    //初始化log
    Log log;
    if (log.init("okbot_log") != 0) {
        std::cout << "init log failed" << std::endl;
        return -1;
    }
    OKBOT_LOG_NOTICE("{}", "log init success.");


    // use a redundant {} to avoid clion syntax inspections error
    OKBOT_LOG_NOTICE("{}", "okbot started.");
	//实例化API
	std::string cn_apiKey		= "";//请到www.okcoin.cn申请。
	std::string cn_secretKey		= "";//请到www.okcoin.cn申请。
	cnapi = new OKCoinWebSocketApiCn(cn_apiKey,cn_secretKey);			//国内站
    // 初始化trade
    auto& trade = Singleton<Trade>::instance();
    trade.init(cnapi->pWebsocket,
               cn_apiKey,
               cn_secretKey
    );

	cnapi->SetCallBackOpen(cn_callbak_open);
	cnapi->SetCallBackClose(cn_callbak_close);
	cnapi->SetCallBackMessage(cn_callbak_message);
	cnapi->Run();//启动连接服务器线程

    std::this_thread::sleep_for(std::chrono::seconds(2));
    // 初始化全局交易状态机
    Singleton<AccountStatus>::instance();

    // 开启全局资产风险控制线程
//    auto& assetFreeze = Singleton<AssetFreeze>::instance();
//    double play_amount = 100;
//    assetFreeze.set_global_security(play_amount, -0.02);
//    assetFreeze.set_freq(500);
//    assetFreeze.run();

    double play_amount = 100;
    RocStrategy roc_strategy;
    roc_strategy.set_interval(500);
    roc_strategy.set_asset_and_security(play_amount, -0.06);
    roc_strategy.run();

    std::cout << "输入1订阅比特币当周合约行情，" << std::endl <<
              "输入2订阅比特币期货指数，" << std::endl <<
              "输入3取消订阅，" << std::endl <<
              "输入0关闭连接" << std::endl;

    int i;
    while(std::cin >> i) {
        if (cnapi != 0) {
            switch (i) {
                case 1:
                    //trade.buy_at_market(123.4);
                    break;
                case 2:

                    break;
                case 3:
                    cnapi->remove_ok_spotcny_btc_ticker();
                case 0:
                    goto out_loop;
                default:
                    ;
            }
        } else {
            OKBOT_LOG_FATAL("cnapi is nullptr and okbot quit with user input {}.", i);
        };
    }
    out_loop:
	//关闭连接
    //assetFreeze.stop();
    roc_strategy.stop();
    //assetFreeze.join();
    roc_strategy.join();
    cnapi->Close();
	//释放API实例
	//delete cnapi;
	delete cnapi;

    OKBOT_LOG_NOTICE("{}", "okbot stopped.");
}
