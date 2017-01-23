//
// Created by liaosiwei on 17/1/22.
//

#ifndef OKCOIN_BOT_REST_CLIENT_H
#define OKCOIN_BOT_REST_CLIENT_H

#include <memory>
#include <rest_api/net/http/curl/okcoinapi.h>

class RestClient : public OKCoinApi {
public:
    RestClient() {
        urlprotocol.InitApi(HTTP_SERVER_TYPE_CN);
        SetKey(api_key_,secret_key_);
    }

    static std::shared_ptr<RestClient> make_rest_client() {
        return std::shared_ptr<RestClient>(new RestClient);
    }
private:
    // set your api_key and secret_key
    std::string api_key_ = "";
    std::string secret_key_ = "";
};

#endif //OKCOIN_BOT_REST_CLIENT_H
