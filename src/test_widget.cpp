/**
 * @file test_widget.cpp
 * @brief Widget 控件单元测试
 */

#include <gtest/gtest.h>
#include "Component/Widget.hpp"
#include "Component/Types.hpp"

using namespace Component;

/**
 * @brief Widget 基础属性测试
 */
TEST(WidgetTest, BasicProperties) {
    Widget widget;
    
    // 测试初始位置
    auto pos = widget.getPosition();
    EXPECT_FLOAT_EQ(pos.x, 0.0f);
    EXPECT_FLOAT_EQ(pos.y, 0.0f);
    
    // 测试初始大小
    auto size = widget.getSize();
    EXPECT_FLOAT_EQ(size.width, 0.0f);
    EXPECT_FLOAT_EQ(size.height, 0.0f);
    
    // 测试默认可见性
    EXPECT_TRUE(widget.isVisible());
    
    // 测试默认 dirty 状态
    EXPECT_FALSE(widget.needsRender());
}

/**
 * @brief Widget 位置设置测试
 */
TEST(WidgetTest, PositionSetting) {
    Widget widget;
    
    widget.setPosition(100.0f, 200.0f);
    
    auto pos = widget.getPosition();
    EXPECT_FLOAT_EQ(pos.x, 100.0f);
    EXPECT_FLOAT_EQ(pos.y, 200.0f);
    
    // 设置位置后应该标记为 dirty
    EXPECT_TRUE(widget.needsRender());
}

/**
 * @brief Widget 大小设置测试
 */
TEST(WidgetTest, SizeSetting) {
    Widget widget;
    
    widget.setSize(300.0f, 400.0f);
    
    auto size = widget.getSize();
    EXPECT_FLOAT_EQ(size.width, 300.0f);
    EXPECT_FLOAT_EQ(size.height, 400.0f);
    
    // 设置大小后应该标记为 dirty
    EXPECT_TRUE(widget.needsRender());
}

/**
 * @brief Widget 可见性测试
 */
TEST(WidgetTest, Visibility) {
    Widget widget;
    
    // 默认可见
    EXPECT_TRUE(widget.isVisible());
    
    // 设置为不可见
    widget.setVisible(false);
    EXPECT_FALSE(widget.isVisible());
    
    // 重新设置为可见
    widget.setVisible(true);
    EXPECT_TRUE(widget.isVisible());
}

/**
 * @brief Widget ID 测试
 */
TEST(WidgetTest, IdSetting) {
    Widget widget;
    
    widget.setId("test_widget_001");
    
    EXPECT_EQ(widget.getId(), "test_widget_001");
}

/**
 * @brief Widget Z 序测试
 */
TEST(WidgetTest, ZIndex) {
    Widget widget;
    
    // 默认 Z 序
    EXPECT_EQ(widget.getZIndex(), 0);
    
    // 设置 Z 序
    widget.setZIndex(10);
    EXPECT_EQ(widget.getZIndex(), 10);
}

/**
 * @brief Widget 父子关系测试
 */
TEST(WidgetTest, ParentChildRelationship) {
    auto parent = std::make_shared<Widget>();
    auto child = std::make_shared<Widget>();
    
    parent->setId("parent");
    child->setId("child");
    
    // 添加子控件
    parent->addChild(child);
    
    // 验证子控件列表
    const auto& children = parent->getChildren();
    EXPECT_EQ(children.size(), 1u);
    EXPECT_EQ(children[0]->getId(), "child");
}

/**
 * @brief Color 类型测试
 */
TEST(WidgetTest, ColorType) {
    // 测试 RGB 构造
    Color color1(1.0f, 0.5f, 0.0f);
    EXPECT_FLOAT_EQ(color1.r, 1.0f);
    EXPECT_FLOAT_EQ(color1.g, 0.5f);
    EXPECT_FLOAT_EQ(color1.b, 0.0f);
    EXPECT_FLOAT_EQ(color1.a, 1.0f);  // 默认 alpha
    
    // 测试 RGBA 构造
    Color color2(0.5f, 0.5f, 0.5f, 0.5f);
    EXPECT_FLOAT_EQ(color2.a, 0.5f);
    
    // 测试十六进制颜色解析
    Color color3 = Color::fromHex("#FF0000");
    EXPECT_FLOAT_EQ(color3.r, 1.0f);
    EXPECT_FLOAT_EQ(color3.g, 0.0f);
    EXPECT_FLOAT_EQ(color3.b, 0.0f);
    
    // 测试不带#的十六进制
    Color color4 = Color::fromHex("00FF00");
    EXPECT_FLOAT_EQ(color4.r, 0.0f);
    EXPECT_FLOAT_EQ(color4.g, 1.0f);
    EXPECT_FLOAT_EQ(color4.b, 0.0f);
}

/**
 * @brief Vec2 类型测试
 */
TEST(WidgetTest, Vec2Type) {
    Vec2 v1;
    EXPECT_FLOAT_EQ(v1.x, 0.0f);
    EXPECT_FLOAT_EQ(v1.y, 0.0f);
    
    Vec2 v2(10.0f, 20.0f);
    EXPECT_FLOAT_EQ(v2.x, 10.0f);
    EXPECT_FLOAT_EQ(v2.y, 20.0f);
}

/**
 * @brief Rect 类型测试
 */
TEST(WidgetTest, RectType) {
    Rect rect(0.0f, 0.0f, 100.0f, 100.0f);
    
    EXPECT_FLOAT_EQ(rect.x, 0.0f);
    EXPECT_FLOAT_EQ(rect.y, 0.0f);
    EXPECT_FLOAT_EQ(rect.width, 100.0f);
    EXPECT_FLOAT_EQ(rect.height, 100.0f);
}
