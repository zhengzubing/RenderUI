/* GTEST DISABLED - src/test_event.cpp */
/**
 * @file test_event.cpp
 * @brief 事件系统单元测试
 */

// #include <gtest/gtest.h>
#include "EventLoop.hpp"
#include "EventDispatcher.hpp"

using namespace Component;

/**
 * @brief 事件类型测试
 */
TEST(EventTest, EventTypes) {
    // 测试键盘事件
    KeyEvent keyEvent;
    EXPECT_EQ(keyEvent.type, EventType::KeyDown);
    keyEvent.keyCode = 65;  // 'A'
    EXPECT_EQ(keyEvent.keyCode, 65u);
    
    // 测试触摸事件
    TouchEvent touchEvent;
    EXPECT_EQ(touchEvent.type, EventType::TouchDown);
    touchEvent.x = 100.0f;
    touchEvent.y = 200.0f;
    EXPECT_FLOAT_EQ(touchEvent.x, 100.0f);
    EXPECT_FLOAT_EQ(touchEvent.y, 200.0f);
}

/**
 * @brief 事件循环初始化测试
 */
TEST(EventLoopTest, Initialization) {
    auto& loop = EventLoop::instance();
    
    // 测试初始化
    bool result = loop.init();
    EXPECT_TRUE(result);
    
    // 测试再次初始化（应该返回 true）
    result = loop.init();
    EXPECT_TRUE(result);
    
    loop.cleanup();
}

/**
 * @brief 事件循环事件推送测试
 */
TEST(EventLoopTest, EventPosting) {
    auto& loop = EventLoop::instance();
    loop.init();
    
    // 创建并推送事件
    auto event = std::make_unique<KeyEvent>();
    event->keyCode = 65;
    
    loop.postEvent(std::move(event));
    
    // 注意：这里无法直接验证队列内容，因为队列是私有的
    // 实际测试需要通过运行事件循环来验证
    
    loop.cleanup();
}

/**
 * @brief 事件循环处理器注册测试
 */
TEST(EventLoopTest, EventHandlerRegistration) {
    auto& loop = EventLoop::instance();
    loop.init();
    
    bool handlerCalled = false;
    
    // 注册键盘事件处理器
    loop.addHandler(EventType::KeyDown, [&handlerCalled](const Event& event) {
        handlerCalled = true;
    });
    
    // 注意：无法直接触发处理器，需要通过事件循环
    
    loop.cleanup();
}

/**
 * @brief 事件分发器初始化测试
 */
TEST(EventDispatcherTest, Initialization) {
    auto& dispatcher = EventDispatcher::instance();
    
    bool result = dispatcher.init();
    EXPECT_TRUE(result);
    
    // 测试再次初始化
    result = dispatcher.init();
    EXPECT_TRUE(result);
}

/**
 * @brief 事件分发器 WidgetTree 设置测试
 */
TEST(EventDispatcherTest, WidgetTreeSetting) {
    auto& dispatcher = EventDispatcher::instance();
    dispatcher.init();
    
    WidgetTree tree;
    dispatcher.setWidgetTree(&tree);
    
    // 无法直接验证，但确保不崩溃
    SUCCEED();
}

/**
 * @brief 触摸事件坐标测试
 */
TEST(TouchEventTest, Coordinates) {
    TouchEvent event;
    
    event.x = 150.5f;
    event.y = 250.5f;
    event.touchId = 1;
    
    EXPECT_FLOAT_EQ(event.x, 150.5f);
    EXPECT_FLOAT_EQ(event.y, 250.5f);
    EXPECT_EQ(event.touchId, 1);
}

/**
 * @brief 键盘事件键码测试
 */
TEST(KeyEventTest, KeyCode) {
    KeyEvent event;
    
    event.keyCode = 13;  // Enter
    event.modifiers = 0;
    
    EXPECT_EQ(event.keyCode, 13u);
    EXPECT_EQ(event.modifiers, 0u);
    
    // 测试组合键
    event.modifiers = 0x0002;  // Shift
    EXPECT_EQ(event.modifiers, 0x0002u);
}
