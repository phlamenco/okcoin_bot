
#ifndef __OKCOINWEBSOCKETAPI_H__
#define __OKCOINWEBSOCKETAPI_H__
#include <string>



#include "websocket.h"

#define URL_CN						"wss://real.okcoin.cn:10440/websocket/okcoinapi"
#define URL_COM						"wss://real.okcoin.com:10440/websocket/okcoinapi"

class OKCoinWebSocketApi
{

protected:
	OKCoinWebSocketApi();
	void SetKey(std::string api_key,std::string secret_key);
	void SetUri(std::string uri);
	std::string m_api_key;			//用户申请的apiKey
	std::string m_secret_key;		//请求参数签名的私钥
	std::string m_uri;

	void Emit(const char *channel,std::string &parameter);
	void Emit(const char *channel);
	void Emit(std::string &channel);
	void Remove(std::string channel);
private:
	websocketpp_callbak_open		m_callbak_open;
	websocketpp_callbak_close		m_callbak_close;
	websocketpp_callbak_message		m_callbak_message;

public:
	virtual ~OKCoinWebSocketApi();
	void Run();
	void Close();

    static const unsigned retry_count = 4;
	static unsigned RunThread( void* arg );

	WebSocket *pWebsocket;
	std::thread hThread;

	void SetCallBackOpen(websocketpp_callbak_open callbak_open);
	void SetCallBackClose(websocketpp_callbak_close callbak_close);
	void SetCallBackMessage(websocketpp_callbak_message callbak_message);
};

class OKCoinWebSocketApiCn:public OKCoinWebSocketApi //国内站
{
public:
	OKCoinWebSocketApiCn(std::string api_key,std::string secret_key)
	{
		SetKey(api_key,secret_key);
		SetUri(URL_CN); //国内站
	};
	~OKCoinWebSocketApiCn(){};

	//获取OKCoin现货行情数据
	void ok_spotcny_btc_ticker();				//比特币行情数据
	void ok_spotcny_btc_depth_20();				//比特币20条市场深度
	void ok_spotcny_btc_trades();				//比特币成交记录
	void ok_spotcny_btc_kline_1min();			//比特币一分钟K线数据
    void ok_spotcny_btc_kline_5min();			//比特币一分钟K线数据

	//用OKCoin进行交易
	void ok_spotcny_trades();				//实时交易数据
	void ok_spotcny_trade(std::string &symbol,std::string &type,std::string &price,std::string &amount);	//下单交易
	void ok_spotcny_cancel_order(std::string &symbol,std::string &order_id);				//取消订单

	//注销请求
	void remove_ok_spotcny_btc_ticker();			//比特币行情数据
};


class OKCoinWebSocketApiCom:public OKCoinWebSocketApi //国际站
{
public:
	OKCoinWebSocketApiCom(std::string api_key,std::string secret_key)
	{
		SetKey(api_key, secret_key);
		SetUri(URL_COM); //国际站
	};
	~OKCoinWebSocketApiCom(){};

	//获取OKCoin现货行情数据
	void ok_spotusd_btc_ticker();				//比特币行情数据

	//获取OKCoin合约行情数据
	void ok_futureusd_btc_ticker_this_week();		//比特币当周合约行情
	void ok_futureusd_btc_index();				//比特币合约指数
	
	//注销请求
	void remove_ok_spotusd_btc_ticker();			//比特币行情数据
};

#endif /* __OKCOINWEBSOCKETAPI_H__ */
