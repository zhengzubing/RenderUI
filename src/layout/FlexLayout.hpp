#pragma once

#include "Component/Widget.hpp"
#include "Component/LayoutTypes.hpp"
#include <vector>

namespace Component {

/**
 * @brief Flexbox 布局管理器
 * 
 * 实现类似 CSS Flexbox 的弹性布局
 */
class FlexLayout {
public:
    /**
     * @brief 布局控件树
     * @param root 根控件
     * @param parentX 父容器 X 坐标
     * @param parentY 父容器 Y 坐标
     * @param parentWidth 父容器宽度
     * @param parentHeight 父容器高度
     * @param direction 布局方向
     */
    static void layout(Widget* root,
                       float parentX, float parentY,
                       float parentWidth, float parentHeight,
                       LayoutDirection direction = LayoutDirection::Horizontal);
    
    /**
     * @brief 计算子控件大小
     */
    static void measureChildren(Widget* root, float availableWidth, float availableHeight);
    
private:
    struct FlexItem {
        Widget* widget = nullptr;
        float mainSize = 0;      // 主轴大小
        float crossSize = 0;     // 交叉轴大小
        float flexGrow = 0;
        float flexShrink = 1;
        float marginLeft = 0, marginRight = 0;
        float marginTop = 0, marginBottom = 0;
    };
    
    /**
     * @brief 收集 Flex 项目
     */
    static std::vector<FlexItem> collectFlexItems(Widget* root);
    
    /**
     * @brief 计算主轴大小
     */
    static float calculateMainSize(const std::vector<FlexItem>& items, bool isHorizontal);
    
    /**
     * @brief 分配剩余空间
     */
    static void distributeSpace(std::vector<FlexItem>& items, 
                                float availableSpace, bool isHorizontal);
};

} // namespace Component
