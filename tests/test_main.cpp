/**
 * @file main.cpp
 * @brief 主程序入口点
 */

#include <gtest/gtest.h>
#include <iostream>

int main(int argc, char** argv) {
    // 初始化 GoogleTest
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "========================================" << std::endl;
    std::cout << "RenderUI Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // 运行所有测试
    int result = RUN_ALL_TESTS();
    
    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    if (result == 0) {
        std::cout << "✅ All tests passed!" << std::endl;
    } else {
        std::cout << "❌ Some tests failed!" << std::endl;
    }
    std::cout << "========================================" << std::endl;
    
    return result;
}
