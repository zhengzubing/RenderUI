#pragma once

#include "Widget.hpp"
#include "LayoutTypes.hpp"
#include <memory>

namespace Component {

/**
 * @brief 绝对布局管理器
 * 
 * 控件使用绝对坐标定位
 */
class AbsoluteLayout {
public:
    /**
     * @brief 布局控件树
     * @param root 根控件
     * @param parentX 父容器 X 坐标
     * @param parentY 父容器 Y 坐标
     * @param parentWidth 父容器宽度
     * @param parentHeight 父容器高度
     */
    static void layout(Widget* root, 
                       float parentX, float parentY,
                       float parentWidth, float parentHeight);
    
    /**
     * @brief 设置控件位置（绝对坐标）
     */
    static void setPosition(Widget* widget, float x, float y);
    
    /**
     * @brief 设置控件大小
     */
    static void setSize(Widget* widget, float width, float height);
};

} // namespace Component
