#include "Component/AbsoluteLayout.hpp"

namespace Component {

void AbsoluteLayout::layout(Widget* root,
                            float parentX, float parentY,
                            float parentWidth, float parentHeight) {
    if (!root) {
        return;
    }
    
    // 获取控件当前位置和大小
    auto pos = root->getPosition();
    auto size = root->getSize();
    
    // 如果是百分比大小，转换为绝对值
    if (size.width > 0 && size.width <= 1.0f) {
        size.width = parentWidth * size.width;
    }
    if (size.height > 0 && size.height <= 1.0f) {
        size.height = parentHeight * size.height;
    }
    
    // 如果是百分比位置，转换为绝对值
    if (pos.x > 0 && pos.x <= 1.0f) {
        pos.x = parentX + parentWidth * pos.x;
    } else {
        pos.x = parentX + pos.x;
    }
    
    if (pos.y > 0 && pos.y <= 1.0f) {
        pos.y = parentY + parentHeight * pos.y;
    } else {
        pos.y = parentY + pos.y;
    }
    
    // 应用位置和大小
    root->setPosition(pos.x, pos.y);
    if (size.width > 0 && size.height > 0) {
        root->setSize(size.width, size.height);
    }
    
    // 递归布局子控件
    auto& children = root->getChildren();
    for (auto& child : children) {
        layout(child.get(), parentX, parentY, parentWidth, parentHeight);
    }
}

void AbsoluteLayout::setPosition(Widget* widget, float x, float y) {
    if (widget) {
        widget->setPosition(x, y);
    }
}

void AbsoluteLayout::setSize(Widget* widget, float width, float height) {
    if (widget) {
        widget->setSize(width, height);
    }
}

} // namespace Component
