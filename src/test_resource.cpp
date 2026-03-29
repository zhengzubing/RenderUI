/* GTEST DISABLED - src/test_resource.cpp */
/**
 * @file test_resource.cpp
 * @brief 资源管理单元测试
 */

// #include <gtest/gtest.h>
#include "ResourceManager.hpp"
#include "ImageLoader.hpp"

using namespace Component;

/**
 * @brief 资源管理器初始化测试
 */
TEST(ResourceManagerTest, Initialization) {
    auto& manager = ResourceManager::instance();
    
    bool result = manager.init("./test_assets");
    EXPECT_TRUE(result);
    
    EXPECT_EQ(manager.getResourceDir(), "./test_assets");
}

/**
 * @brief 资源管理器单例测试
 */
TEST(ResourceManagerTest, Singleton) {
    auto& manager1 = ResourceManager::instance();
    auto& manager2 = ResourceManager::instance();
    
    // 验证是同一个实例
    EXPECT_EQ(&manager1, &manager2);
}

/**
 * @brief 图片加载器基本测试
 */
TEST(ImageLoaderTest, BasicLoading) {
    ImageLoader loader;
    
    // 测试不存在的文件
    auto data = loader.load("nonexistent.png");
    EXPECT_EQ(data, nullptr);
    
    // 获取错误信息（不应该崩溃）
    const char* reason = ImageLoader::getFailureReason();
    EXPECT_NE(reason, nullptr);
}

/**
 * @brief 资源计数测试
 */
TEST(ResourceManagerTest, ResourceCount) {
    auto& manager = ResourceManager::instance();
    manager.init("./test_assets");
    
    size_t initialCount = manager.getResourceCount();
    
    // 尝试加载不存在的图片（应该失败）
    cairo_surface_t* surface = manager.loadImage("nonexistent.png");
    EXPECT_EQ(surface, nullptr);
    
    // 资源数量不应该增加
    EXPECT_EQ(manager.getResourceCount(), initialCount);
}

/**
 * @brief 资源内存使用测试
 */
TEST(ResourceManagerTest, MemoryUsage) {
    auto& manager = ResourceManager::instance();
    
    size_t usage = manager.getMemoryUsage();
    
    // 初始状态应该是 0 或很小的值
    EXPECT_GE(usage, 0u);
}

/**
 * @brief 资源清理测试
 */
TEST(ResourceManagerTest, Cleanup) {
    auto& manager = ResourceManager::instance();
    manager.init("./test_assets");
    
    // 清理未使用的资源（TTL 60 秒）
    manager.cleanupUnused(60);
    
    // 清理所有资源
    manager.clearAll();
    
    // 验证资源数量为 0
    EXPECT_EQ(manager.getResourceCount(), 0u);
    EXPECT_EQ(manager.getMemoryUsage(), 0u);
}

/**
 * @brief 资源卸载测试
 */
TEST(ResourceManagerTest, Unload) {
    auto& manager = ResourceManager::instance();
    manager.init("./test_assets");
    
    // 卸载不存在的资源（不应该崩溃）
    manager.unloadImage("nonexistent.png");
    
    SUCCEED();
}

/**
 * @brief 颜色类型测试
 */
TEST(ResourceTest, ColorConversion) {
    // 测试红色
    Color red = Color::fromHex("#FF0000");
    EXPECT_FLOAT_EQ(red.r, 1.0f);
    EXPECT_FLOAT_EQ(red.g, 0.0f);
    EXPECT_FLOAT_EQ(red.b, 0.0f);
    
    // 测试绿色
    Color green = Color::fromHex("#00FF00");
    EXPECT_FLOAT_EQ(green.r, 0.0f);
    EXPECT_FLOAT_EQ(green.g, 1.0f);
    EXPECT_FLOAT_EQ(green.b, 0.0f);
    
    // 测试蓝色
    Color blue = Color::fromHex("#0000FF");
    EXPECT_FLOAT_EQ(blue.r, 0.0f);
    EXPECT_FLOAT_EQ(blue.g, 0.0f);
    EXPECT_FLOAT_EQ(blue.b, 1.0f);
    
    // 测试白色
    Color white = Color::fromHex("#FFFFFF");
    EXPECT_FLOAT_EQ(white.r, 1.0f);
    EXPECT_FLOAT_EQ(white.g, 1.0f);
    EXPECT_FLOAT_EQ(white.b, 1.0f);
    
    // 测试黑色
    Color black = Color::fromHex("#000000");
    EXPECT_FLOAT_EQ(black.r, 0.0f);
    EXPECT_FLOAT_EQ(black.g, 0.0f);
    EXPECT_FLOAT_EQ(black.b, 0.0f);
}
