//
// Created by liaosiwei on 17/1/6.
//

#include <boost/regex.hpp>
#include "util/json_util.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace json_util {

    int parseKey(std::string &key) {
        boost::smatch what;
        boost::regex expr{"(\\\\\\d+$)"};

        if (boost::regex_search(key, what, expr)) {
            std::string index(what[0].first, what[0].second);
            key.erase(key.size() - index.size());
            return stoi(index.substr(1));
        }
        return -1;
    }

    rapidjson::Value *get_node(rapidjson::Value *node, const std::string &path) {
        if (path.empty())
            return node;

        if (!node->IsObject())
            return nullptr;

        size_t index = path.find_first_of(".");
        std::string key = path.substr(0, index);
        unsigned long start;
        if (index == -1)
            start = key.size();
        else
            start = key.size() + 1;
        int which = parseKey(key);

        auto itr = node->FindMember(key.c_str());
        if (itr == node->MemberEnd())
            return nullptr;
        else {
            if (which != -1) {
                if (!itr->value.IsArray())
                    return nullptr;
                else {
                    if (which < itr->value.Size() && which >= 0)
                        return get_node(&((itr->value)[which]), path.substr(start));
                    else
                        return nullptr;
                }
            } else {
                return get_node(&(itr->value), path.substr(start));
            }
        }
    }

    std::string json_to_str(rapidjson::Value *node, rapidjson::Document::AllocatorType &alloc) {
        using namespace rapidjson;
        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        node->Accept(writer);
        return sb.GetString();
    }

    rapidjson::Value* get_data(rapidjson::Document* d, const std::string& channel_name) {
        if (d->IsArray()) {
            for (auto i = 0; i < d->Size(); i++) {
                auto& node = (*d)[i];
                std::string channel;
                get_value(&node, "channel", channel);
                if (channel == channel_name) {
                    return get_node(&node, "data");
                }
            }
        }
        return nullptr;
    }
}