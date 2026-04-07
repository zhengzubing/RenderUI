#include "Container.hpp"
#include "Logger.hpp"
#include "RenderContext.hpp"

namespace Component {

void Container::Add(std::shared_ptr<Widget> widget, const std::string& id) {
    if (!widget) {
        return;
    }
    
    LOG_I << "Adding widget to container: " << widget->getId();

    // 如果指定了 ID，使用指定的 ID
    if (!id.empty()) {
        widget->setId(id);
    }
    
    // 添加到列表和映射
    children_.push_back(widget);
    childMap_[widget->getId()] = widget;
    
    // 设置父关系（通过 WidgetTree 或父容器管理）
    // 这里不直接设置，避免循环引用
    
    markDirty();
}

void Container::Remove(const std::string& id) {
    // 1. 从映射表中查找
    auto it = childMap_.find(id);
    if (it == childMap_.end()) {
        return;  // 未找到，直接返回
    }
    
    // 2. 获取要删除的控件引用
    auto widgetToRemove = it->second;
    
    // 3. 从列表中移除（使用 C++20 的 std::erase_if）
    std::erase_if(children_, [&widgetToRemove](const auto& child) {
        return child == widgetToRemove;
    });
    
    // 4. 从映射表中移除
    childMap_.erase(it);
    
    // 5. 标记为脏，触发重绘
    markDirty();
}

std::shared_ptr<Widget> Container::Get(const std::string& id) const {
    auto it = childMap_.find(id);
    return (it != childMap_.end()) ? it->second : nullptr;
}

void Container::render(RenderContext& ctx) {
    if (!isVisible()) {
        return;
    }
    
    // 渲染容器本身（如果有绘制逻辑）
    Widget::render(ctx);
    
    // 渲染所有子控件
    for (auto& child : children_) {
        if (child && child->isVisible()) {
            child->render(ctx);
        }
    }
}

bool Container::handleTouchEvent(const TouchEvent& event) {
    // 先让子控件处理事件（从后往前，处理最上层的控件）
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        auto& child = *it;
        if (child && child->isVisible()) {
            if (child->handleTouchEvent(event)) {
                return true;
            }
        }
    }
    
    // 子控件都不处理，则由容器自己处理
    return Widget::handleTouchEvent(event);
}

void Container::fromJson(const json& config) {
    // 先调用基类处理公共属性
    Widget::fromJson(config);
}

} // namespace Component
