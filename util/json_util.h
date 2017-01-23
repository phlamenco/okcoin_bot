//
// Created by liaosiwei on 17/1/6.
//

#ifndef OKCOIN_BOT_JSON_UTIL_H
#define OKCOIN_BOT_JSON_UTIL_H

#include <type_traits>
#include <string>
#include <vector>
#include "rapidjson/document.h"
#include <boost/algorithm/string.hpp>

namespace json_util {
// for int
template <typename T>
typename std::enable_if<std::is_same<T, int>::value, void>::type
    _set_value(rapidjson::Value* node, T value, rapidjson::Document::AllocatorType& d)
{
    node->SetInt(value);
}

template <typename T>
typename std::enable_if<std::is_same<T, int>::value, int>::type
    _get_value(rapidjson::Value* node, T& value)
{
    if (node->IsInt()) {
        value = node->GetInt();
        return 0;
    } else if (node->IsString()){
        try {
            value = std::stoi(node->GetString());
            return 0;
        } catch(...) {
            return -1;
        }
    } else {
        return -1;
    }
}

// int64_t
template <typename T>
typename std::enable_if<std::is_same<T, int64_t>::value, void>::type
    _set_value(rapidjson::Value* node, T value, rapidjson::Document::AllocatorType& d)
{
    node->SetInt64(value);
}

template <typename T>
typename std::enable_if<std::is_same<T, int64_t>::value, int64_t>::type
    _get_value(rapidjson::Value* node, T& value)
{
    if (node->IsInt64()) {
        value = node->GetInt64();
        return 0;
    } else if (node->IsString()){
        try {
            value = std::stoll(node->GetString());
            return 0;
        } catch(...) {
            return -1;
        }
    } else {
        return -1;
    }
}

// for string
template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value, void>::type
    _set_value(rapidjson::Value* node, T value, rapidjson::Document::AllocatorType& d)
{
    node->SetString(value.c_str(), value.length(), d);
}

template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value, int>::type
    _get_value(rapidjson::Value* node, T& value)
{
    if (node->IsString()) {
        value = std::string(node->GetString());
        return 0;
    } else if (node->IsInt()) {
        value = std::to_string(node->GetInt());
        return 0;
    } else if (node->IsInt64()) {
        value = std::to_string(node->GetInt64());
        return 0;
    } else {
        return -1;
    }
}

template <class T>
typename std::enable_if<std::is_same<T, double>::value, void>::type
_set_value(rapidjson::Value*node, T value, rapidjson::Document::AllocatorType& d)
{
    node->SetDouble(value);
}

template <typename T>
typename std::enable_if<std::is_same<T, double>::value, int>::type
    _get_value(rapidjson::Value* node, T& value)
{
    if (node->IsDouble()) {
        value = node->GetDouble();
        return 0;
    } else if (node->IsString()){
        try {
            value = std::stod(node->GetString());
            return 0;
        } catch(...) {
            return -1;
        }
    } else if (node->IsInt()) {
        value = node->GetInt();
        return 0;
    } else if (node->IsInt64()) {
        value = node->GetInt64();
        return 0;
    } else {
        return -1;
    }
}

// for char *
template <typename T>
typename std::enable_if<std::is_same<T, const char *>::value, void>::type
    _set_value(rapidjson::Value* node, T value, rapidjson::Document::AllocatorType& d)
{
    node->SetString(value, d);
}

//template <typename T>
//typename std::enable_if<std::is_same<T, char *>::value, void>::type
//     _get_value(rapidjson::Value* node, T& value)
// {
//     value = node->GetString();
// }

rapidjson::Value* get_node(rapidjson::Value* node, const std::string& path);

template <typename T>
int get_value(rapidjson::Value* node,
                   const std::string& key,
                   T& value) {
    if (key.empty())
        return -1;
    auto new_node = get_node(node, key);
    if (new_node != nullptr) {
        return _get_value(new_node, value);
    }
    return -1;
}
template <typename T>
int set_value(rapidjson::Value* node,
              const std::string& key,
              T& value,
              rapidjson::Document::AllocatorType& d) {
    if (key.empty())
        return -1;
    auto new_node = get_node(node, key);
    if (new_node != nullptr) {
        _set_value(new_node, value, d);
        return 0;
    }
    return -1;
}

std::string json_to_str(rapidjson::Value* node, rapidjson::Document::AllocatorType& alloc);

    rapidjson::Value* get_data(rapidjson::Document*, const std::string& channel_name);

}

#endif //OKCOIN_BOT_JSON_UTIL_H
