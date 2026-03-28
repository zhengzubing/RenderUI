/**
 * @file main.cpp
 * @brief 测试入口点
 */

#include <gtest/gtest.h>
#include <iostream>

/**
 * @brief 测试前初始化
 */
void SetUpTestSuite() {
    std::cout << "=== RenderUI Test Suite Started ===" << std::endl;
}

/**
 * @brief 测试后清理
 */
void TearDownTestSuite() {
    std::cout << "=== RenderUI Test Suite Finished ===" << std::endl;
}

int main(int argc, char** argv) {
    // 初始化 GoogleTest
    ::testing::InitGoogleTest(&argc, argv);
    
    // 设置测试套件
    SetUpTestSuite();
    
    // 运行所有测试
    int result = RUN_ALL_TESTS();
    
    // 清理
    TearDownTestSuite();
    
    return result;
}
