/**
 * @file test_basic.cpp
 * @brief 基础功能测试示例
 */

#include <gtest/gtest.h>
#include <iostream>
#include <memory>

// 简单的加法函数测试
TEST(BasicTest, Addition) {
    EXPECT_EQ(2 + 3, 5);
    EXPECT_EQ(10 + 20, 30);
    EXPECT_EQ(-1 + 1, 0);
}

// 简单的减法函数测试
TEST(BasicTest, Subtraction) {
    EXPECT_EQ(5 - 3, 2);
    EXPECT_EQ(100 - 50, 50);
}

// 测试智能指针
TEST(BasicTest, SmartPointer) {
    auto ptr = std::make_shared<int>(42);
    
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);
}

// 测试字符串
TEST(BasicTest, StringOperations) {
    std::string str1 = "Hello";
    std::string str2 = "World";
    
    EXPECT_STREQ(str1.c_str(), "Hello");
    EXPECT_NE(str1, str2);
    EXPECT_EQ(str1 + " " + str2, "Hello World");
}

// 测试布尔值
TEST(BasicTest, BooleanLogic) {
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_TRUE(1 == 1);
    EXPECT_FALSE(1 == 2);
}

// 使用 Test Fixture 的示例
class MathTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前执行
        result_ = 0;
    }
    
    void TearDown() override {
        // 每个测试后执行
    }
    
    int result_;
};

TEST_F(MathTest, Multiply) {
    result_ = 6 * 7;
    EXPECT_EQ(result_, 42);
}

TEST_F(MathTest, Divide) {
    result_ = 100 / 4;
    EXPECT_EQ(result_, 25);
}
