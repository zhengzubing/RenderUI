#pragma once

namespace Component {

/**
 * @brief 布局方向
 */
enum class LayoutDirection {
    Horizontal,  // 水平排列
    Vertical     // 垂直排列
};

/**
 * @brief 对齐方式
 */
enum class Alignment {
    Left,
    Right,
    Top,
    Bottom,
    Center,
    Start,
    End
};

/**
 * @brief 布局约束
 */
struct LayoutConstraint {
    float minWidth = 0;
    float maxWidth = -1;  // -1 表示无限制
    float minHeight = 0;
    float maxHeight = -1;
    
    float flexGrow = 0;      // 放大权重
    float flexShrink = 1;    // 缩小权重
    float flexBasis = -1;    // 基础大小 (-1 自动)
    
    float marginLeft = 0;
    float marginRight = 0;
    float marginTop = 0;
    float marginBottom = 0;
    
    bool isWidthFixed = false;
    bool isHeightFixed = false;
};

} // namespace Component
