//
// Created by liaosiwei on 17/1/20.
//

#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include "okcoinapi.h"
#include "rest_api/rest_client.h"


TEST(rest_api, userinfo) {
    auto rest_client = RestClient::make_rest_client();
    //OKCoinApiCn cnapi(cn_apiKey,cn_secretKey);
    std::string symbolcn = "btc_cny";
    std::cout << rest_client->GetTicker(symbolcn) << std::endl;
    std::cout << rest_client->DoUserinfo() << std::endl;
}