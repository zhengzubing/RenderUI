/* GTEST DISABLED - src/test_layout.cpp */
/**
 * @file test_layout.cpp
 * @brief 布局引擎单元测试
 */

// #include <gtest/gtest.h>
#include "AbsoluteLayout.hpp"
#include "FlexLayout.hpp"
#include "Widget.hpp"

using namespace Component;

/**
 * @brief 绝对布局测试
 */
TEST(LayoutTest, AbsoluteLayoutBasic) {
    auto root = std::make_shared<Widget>();
    root->setId("root");
    root->setPosition(0, 0);
    root->setSize(800, 600);
    
    auto child = std::make_shared<Widget>();
    child->setId("child");
    child->setPosition(100, 100);
    child->setSize(200, 150);
    
    root->addChild(child);
    
    // 执行布局
    AbsoluteLayout::layout(root.get(), 0, 0, 800, 600);
    
    // 验证根控件
    auto rootPos = root->getPosition();
    EXPECT_FLOAT_EQ(rootPos.x, 0.0f);
    EXPECT_FLOAT_EQ(rootPos.y, 0.0f);
    
    // 验证子控件位置（应该相对于父容器偏移）
    auto childPos = child->getPosition();
    EXPECT_FLOAT_EQ(childPos.x, 100.0f);
    EXPECT_FLOAT_EQ(childPos.y, 100.0f);
}

/**
 * @brief 绝对布局百分比测试
 */
TEST(LayoutTest, AbsoluteLayoutPercentage) {
    auto root = std::make_shared<Widget>();
    root->setId("root");
    root->setSize(1000, 800);
    
    auto child = std::make_shared<Widget>();
    child->setId("child");
    // 使用百分比（0.5 表示 50%）
    child->setPosition(0.5f, 0.5f);
    child->setSize(0.25f, 0.25f);
    
    root->addChild(child);
    
    // 执行布局
    AbsoluteLayout::layout(root.get(), 0, 0, 1000, 800);
    
    // 验证百分比转换
    auto pos = child->getPosition();
    EXPECT_FLOAT_EQ(pos.x, 500.0f);  // 50% of 1000
    EXPECT_FLOAT_EQ(pos.y, 400.0f);  // 50% of 800
    
    auto size = child->getSize();
    EXPECT_FLOAT_EQ(size.width, 250.0f);   // 25% of 1000
    EXPECT_FLOAT_EQ(size.height, 200.0f);  // 25% of 800
}

/**
 * @brief Flexbox 水平布局测试
 */
TEST(LayoutTest, FlexLayoutHorizontal) {
    auto container = std::make_shared<Widget>();
    container->setId("container");
    container->setSize(600, 100);
    
    auto item1 = std::make_shared<Widget>();
    item1->setId("item1");
    item1->setSize(100, 100);
    
    auto item2 = std::make_shared<Widget>();
    item2->setId("item2");
    item2->setSize(100, 100);
    
    container->addChild(item1);
    container->addChild(item2);
    
    // 执行水平 Flex 布局
    FlexLayout::layout(container.get(), 0, 0, 600, 100, LayoutDirection::Horizontal);
    
    // 验证子控件位置（应该水平排列）
    auto pos1 = item1->getPosition();
    auto pos2 = item2->getPosition();
    
    EXPECT_FLOAT_EQ(pos1.x, 0.0f);
    EXPECT_FLOAT_EQ(pos1.y, 0.0f);
    
    EXPECT_FLOAT_EQ(pos2.x, 100.0f);  // 在 item1 右边
    EXPECT_FLOAT_EQ(pos2.y, 0.0f);
}

/**
 * @brief Flexbox 垂直布局测试
 */
TEST(LayoutTest, FlexLayoutVertical) {
    auto container = std::make_shared<Widget>();
    container->setId("container");
    container->setSize(100, 600);
    
    auto item1 = std::make_shared<Widget>();
    item1->setId("item1");
    item1->setSize(100, 100);
    
    auto item2 = std::make_shared<Widget>();
    item2->setId("item2");
    item2->setSize(100, 100);
    
    container->addChild(item1);
    container->addChild(item2);
    
    // 执行垂直 Flex 布局
    FlexLayout::layout(container.get(), 0, 0, 100, 600, LayoutDirection::Vertical);
    
    // 验证子控件位置（应该垂直排列）
    auto pos1 = item1->getPosition();
    auto pos2 = item2->getPosition();
    
    EXPECT_FLOAT_EQ(pos1.x, 0.0f);
    EXPECT_FLOAT_EQ(pos1.y, 0.0f);
    
    EXPECT_FLOAT_EQ(pos2.x, 0.0f);
    EXPECT_FLOAT_EQ(pos2.y, 100.0f);  // 在 item1 下边
}

/**
 * @brief Flexbox 空间分配测试
 */
TEST(LayoutTest, FlexLayoutSpaceDistribution) {
    auto container = std::make_shared<Widget>();
    container->setId("container");
    container->setSize(300, 100);
    
    auto item1 = std::make_shared<Widget>();
    item1->setId("item1");
    item1->setSize(50, 100);
    
    auto item2 = std::make_shared<Widget>();
    item2->setId("item2");
    item2->setSize(50, 100);
    
    container->addChild(item1);
    container->addChild(item2);
    
    // 执行布局（剩余空间应该被分配）
    FlexLayout::layout(container.get(), 0, 0, 300, 100, LayoutDirection::Horizontal);
    
    // 两个 item 总宽度 100，容器宽度 300，剩余 200
    // 如果 flex-grow 都为 0，则不分配额外空间
    // TODO: 实现 flex-grow 后验证空间分配
}
