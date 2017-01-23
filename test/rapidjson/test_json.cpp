//
// Created by liaosiwei on 17/1/6.
//

#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include "gtest/gtest.h"
#include "util/json_util.h"

using namespace json_util;

TEST(atest_test, self_increase) {
    int i = 0;
    i++;
    EXPECT_EQ(1, i);
}

using namespace rapidjson;

class JsonTest : public ::testing::Test {
public:
    JsonTest() {
        const char* json_str = "{\"project\": {\"name\": \"rapidjson\"},\"stars\":\"10\", \"comma:\": \"test\", \"a\": {\"b\": [1, 2, 3, 4]}, \"a1\": [{\"b1\": [1, 2, 3, 4]}, {\"b1\": [1, 2, 3, 4]}], \"c\": [{\"d1\": 11}, {\"d2\": 22}, {\"d3\": 33}], \"unicode\": \"无结果\"}";
        d.Parse(json_str);
        //output();
    }
    ~JsonTest() override {

    }
    void SetUp() override {

    }
    void TearDown() override {
        output();
    }
    void output() {
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        d.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
        puts(sb.GetString());
    }
    Document d;
};

TEST_F(JsonTest, get_node) {
    std::string name;
    Value* node = get_node(&d, "stars");
    EXPECT_EQ(node != nullptr, true);
    node = get_node(&d, "a.b");
    EXPECT_EQ(node != nullptr, true);
    node = get_node(&d, "a1\\1.b1\\3");
    EXPECT_EQ(node != nullptr, true);
}

TEST_F(JsonTest, get_value) {
    std::string name;
    EXPECT_EQ(0, get_value(&d, "stars", name));
    EXPECT_EQ("10", name);
}

TEST_F(JsonTest, set_value) {
    std::string name = "set_value_1";
    EXPECT_EQ(0, set_value(&d, "stars", name, d.GetAllocator()));
    name = "";
    EXPECT_EQ(0, get_value(&d, "stars", name));
    EXPECT_EQ("set_value_1", name);
    EXPECT_EQ(0, set_value(&d, "a.b\\0", "set_value_2", d.GetAllocator()));
    EXPECT_EQ(0, set_value(&d, "a1\\0.b1\\2", "2", d.GetAllocator()));
}
TEST_F(JsonTest, json_to_str) {
    puts(json_to_str(&d, d.GetAllocator()).c_str());
}