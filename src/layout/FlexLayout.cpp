#include "FlexLayout.hpp"
#include "Container.hpp"
#include <algorithm>

namespace Component {

std::vector<FlexLayout::FlexItem> FlexLayout::collectFlexItems(Widget* root) {
    std::vector<FlexItem> items;
    
    if (!root) {
        return items;
    }
    
    if (auto container = dynamic_cast<Container*>(root)) {
        auto& children = container->GetChildren();
        for (auto& child : children) {
            FlexItem item;
            item.widget = child.get();
            
            auto size = child->getSize();
            item.mainSize = size.width > 0 ? size.width : 0;
            item.crossSize = size.height > 0 ? size.height : 0;
            
            // TODO: 从约束中获取 flex-grow/shrink
            item.flexGrow = 0;
            item.flexShrink = 1;
            
            items.push_back(item);
        }
    }
    
    return items;
}

float FlexLayout::calculateMainSize(const std::vector<FlexItem>& items, bool isHorizontal) {
    float totalSize = 0;
    
    for (const auto& item : items) {
        totalSize += isHorizontal ? item.mainSize : item.crossSize;
        totalSize += item.marginLeft + item.marginRight;
    }
    
    return totalSize;
}

void FlexLayout::distributeSpace(std::vector<FlexItem>& items, 
                                  float availableSpace, bool isHorizontal) {
    if (items.empty() || availableSpace <= 0) {
        return;
    }
    
    // 计算总 flexGrow
    float totalGrow = 0;
    for (const auto& item : items) {
        totalGrow += item.flexGrow;
    }
    
    if (totalGrow > 0) {
        // 按权重分配剩余空间
        for (auto& item : items) {
            float extraSpace = (item.flexGrow / totalGrow) * availableSpace;
            if (isHorizontal) {
                item.mainSize += extraSpace;
            } else {
                item.crossSize += extraSpace;
            }
        }
    }
}

void FlexLayout::measureChildren(Widget* root, float availableWidth, float availableHeight) {
    if (!root) {
        return;
    }
    
    if (auto container = dynamic_cast<Container*>(root)) {
        auto& children = container->GetChildren();
        for (auto& child : children) {
            auto size = child->getSize();
            
            // 如果宽度未设置且父容器有宽度，使用父容器宽度
            if (size.width <= 0 && availableWidth > 0) {
                child->setSize(availableWidth, size.height);
            }
            
            // 如果高度未设置且父容器有高度，使用父容器高度
            if (size.height <= 0 && availableHeight > 0) {
                child->setSize(size.width, availableHeight);
            }
            
            // 递归测量
            measureChildren(child.get(), availableWidth, availableHeight);
        }
    }
}

void FlexLayout::layout(Widget* root,
                        float parentX, float parentY,
                        float parentWidth, float parentHeight,
                        LayoutDirection direction) {
    if (!root) {
        return;
    }
    
    bool isHorizontal = (direction == LayoutDirection::Horizontal);
    
    // 收集子控件
    auto items = collectFlexItems(root);
    if (items.empty()) {
        return;
    }
    
    // 计算已占用空间
    float usedSpace = calculateMainSize(items, isHorizontal);
    float availableSpace = (isHorizontal ? parentWidth : parentHeight) - usedSpace;
    
    // 分配剩余空间
    distributeSpace(items, availableSpace, isHorizontal);
    
    // 应用位置和大小
    float currentPos = isHorizontal ? parentX : parentY;
    
    for (auto& item : items) {
        if (!item.widget) {
            continue;
        }
        
        float x, y, width, height;
        
        if (isHorizontal) {
            x = parentX + currentPos + item.marginLeft;
            y = parentY;
            width = item.mainSize;
            height = parentHeight;
        } else {
            x = parentX;
            y = parentY + currentPos + item.marginTop;
            width = parentWidth;
            height = item.crossSize;
        }
        
        item.widget->setPosition(x, y);
        if (width > 0 && height > 0) {
            item.widget->setSize(width, height);
        }
        
        // 更新位置
        currentPos += isHorizontal ? 
            (item.mainSize + item.marginLeft + item.marginRight) :
            (item.crossSize + item.marginTop + item.marginBottom);
        
        // 递归布局子控件
        layout(item.widget, x, y, width, height, direction);
    }
}

} // namespace Component
