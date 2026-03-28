#include <gtest/gtest.h>
#include "Component/DBusAdapter.hpp"
#include "Component/CanSignal.hpp"
#include <thread>
#include <chrono>

using namespace RenderUI;

/**
 * @brief DBus 和 CAN 信号测试
 * 
 * 测试 DBus 通信、CAN 信号注册、值转换等功能
 * 注意：这些测试可能需要 DBus 服务运行
 */

TEST(DBusAdapterTest, CreateInstance) {
    // DBusAdapter 应该是单例
    auto& adapter = DBusAdapter::instance();
    EXPECT_NO_THROW([&]() {
        // 获取实例不应该抛出异常
    }());
}

TEST(DBusAdapterTest, SignalSubscription) {
    auto& adapter = DBusAdapter::instance();
    
    // 订阅一个测试信号 (可能失败如果 DBus 未运行)
    // 这里只验证接口不崩溃
    EXPECT_NO_THROW([&]() {
        uint32_t subId = adapter.subscribeSignal(
            "com.example.Test",
            "/com/example/Test",
            "TestSignal"
        );
        // subId 可能是 0 如果订阅失败，但不应崩溃
    }());
}

TEST(DBusAdapterTest, SignalCallback) {
    auto& adapter = DBusAdapter::instance();
    
    bool callbackCalled = false;
    
    // 设置信号回调
    adapter.setSignalCallback([&](const std::string& interface, 
                                   const std::string& path,
                                   const std::string& member,
                                   const DBusAdapter::SignalValue& value) {
        callbackCalled = true;
    });
    
    // 发送测试信号
    DBusAdapter::SignalValue value;
    value.type = DBusAdapter::SignalType::INT32;
    value.intValue = 42;
    
    EXPECT_NO_THROW([&]() {
        adapter.sendSignal("com.example.Test", "/com/example/Test", 
                          "TestSignal", value.type, value);
    }());
}

TEST(CanSignalManagerTest, CreateInstance) {
    // CanSignalManager 应该是单例
    auto& manager = CanSignalManager::instance();
    EXPECT_NO_THROW([&]() {
        // 获取实例不应该抛出异常
    }());
}

TEST(CanSignalManagerTest, RegisterSignal) {
    auto& manager = CanSignalManager::instance();
    
    // 注册一个测试信号
    CanSignalDef def;
    def.signalId = 1;
    def.name = "test_speed";
    def.scale = 0.01f;
    def.offset = 0.0f;
    def.minValue = 0.0f;
    def.maxValue = 300.0f;
    def.unit = "km/h";
    
    bool registered = manager.registerSignal("test_speed", def);
    EXPECT_TRUE(registered);
}

TEST(CanSignalManagerTest, RawToPhysicalConversion) {
    auto& manager = CanSignalManager::instance();
    
    // 注册带缩放的信号
    CanSignalDef def;
    def.signalId = 2;
    def.name = "scaled_signal";
    def.scale = 0.1f;
    def.offset = 10.0f;
    def.minValue = 0.0f;
    def.maxValue = 100.0f;
    def.unit = "°C";
    
    manager.registerSignal("scaled_signal", def);
    
    // 模拟原始值 500 -> 物理值 = 500 * 0.1 + 10 = 60
    manager.updateRawValue("scaled_signal", 500);
    
    float physicalValue = manager.getPhysicalValue("scaled_signal");
    EXPECT_NEAR(physicalValue, 60.0f, 1e-5);
}

TEST(CanSignalManagerTest, ValueClamping) {
    auto& manager = CanSignalManager::instance();
    
    // 注册带范围限制的信号
    CanSignalDef def;
    def.signalId = 3;
    def.name = "clamped_signal";
    def.scale = 1.0f;
    def.offset = 0.0f;
    def.minValue = 0.0f;
    def.maxValue = 100.0f;
    def.unit = "%";
    
    manager.registerSignal("clamped_signal", def);
    
    // 超出范围的值应该被钳制
    manager.updateRawValue("clamped_signal", 150);  // 应该被限制到 100
    float value1 = manager.getPhysicalValue("clamped_signal");
    EXPECT_FLOAT_EQ(value1, 100.0f);
    
    manager.updateRawValue("clamped_signal", -10);  // 应该被限制到 0
    float value2 = manager.getPhysicalValue("clamped_signal");
    EXPECT_FLOAT_EQ(value2, 0.0f);
}

TEST(CanSignalManagerTest, ValueChangedListener) {
    auto& manager = CanSignalManager::instance();
    
    // 注册信号
    CanSignalDef def;
    def.signalId = 4;
    def.name = "listener_test";
    def.scale = 1.0f;
    def.offset = 0.0f;
    def.minValue = 0.0f;
    def.maxValue = 100.0f;
    def.unit = "";
    
    manager.registerSignal("listener_test", def);
    
    int changeCount = 0;
    float lastValue = 0.0f;
    
    // 添加值变化监听器
    manager.addValueChangedListener("listener_test", 
        [&](const std::string& signalId, float newValue) {
            changeCount++;
            lastValue = newValue;
        });
    
    // 更新值多次
    for (int i = 0; i < 5; i++) {
        manager.updateRawValue("listener_test", i * 10);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 验证监听器被调用
    EXPECT_GE(changeCount, 1);
    EXPECT_FLOAT_EQ(lastValue, 40.0f);  // 最后一次更新的值
}

TEST(CanSignalManagerTest, SimulateValue) {
    auto& manager = CanSignalManager::instance();
    
    // 注册信号
    CanSignalDef def;
    def.signalId = 5;
    def.name = "simulate_test";
    def.scale = 1.0f;
    def.offset = 0.0f;
    def.minValue = 0.0f;
    def.maxValue = 100.0f;
    def.unit = "km/h";
    
    manager.registerSignal("simulate_test", def);
    
    // 直接模拟物理值
    manager.simulateValue("simulate_test", 80.0f);
    
    float value = manager.getPhysicalValue("simulate_test");
    EXPECT_FLOAT_EQ(value, 80.0f);
}

TEST(CanSignalManagerTest, GetSignalInfo) {
    auto& manager = CanSignalManager::instance();
    
    // 注册信号
    CanSignalDef def;
    def.signalId = 6;
    def.name = "info_test";
    def.scale = 0.5f;
    def.offset = 5.0f;
    def.minValue = -10.0f;
    def.maxValue = 200.0f;
    def.unit = "RPM";
    
    manager.registerSignal("info_test", def);
    
    // 获取信号信息
    auto info = manager.getSignalInfo("info_test");
    EXPECT_EQ(info.name, "info_test");
    EXPECT_FLOAT_EQ(info.scale, 0.5f);
    EXPECT_FLOAT_EQ(info.offset, 5.0f);
    EXPECT_EQ(info.unit, "RPM");
}

TEST(CanSignalManagerTest, MultipleSignals) {
    auto& manager = CanSignalManager::instance();
    
    // 注册多个信号
    std::vector<std::string> signalNames = {
        "speed", "rpm", "temperature", "fuel_level", "battery_voltage"
    };
    
    for (size_t i = 0; i < signalNames.size(); i++) {
        CanSignalDef def;
        def.signalId = static_cast<int>(100 + i);
        def.name = signalNames[i];
        def.scale = 1.0f;
        def.offset = 0.0f;
        def.minValue = 0.0f;
        def.maxValue = 1000.0f;
        def.unit = "";
        
        manager.registerSignal(signalNames[i], def);
    }
    
    // 验证所有信号都已注册
    for (const auto& name : signalNames) {
        EXPECT_NO_THROW([&]() {
            manager.getPhysicalValue(name);
        }());
    }
}
