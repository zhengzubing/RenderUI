#include "WidgetTree.hpp"
#include "Container.hpp"
#include "Logger.hpp"
#include <algorithm>

namespace Component {

WidgetTree::WidgetTree() = default;

WidgetTree::~WidgetTree() = default;

void WidgetTree::addRoot(std::shared_ptr<Widget> widget, const std::string& id) {
    if (!widget) {
        LOG_E << "Cannot add null widget as root";
        return;
    }
    
    auto node = std::make_shared<WidgetNode>();
    node->widget = widget;
    node->id = id;
    node->originalIndex = roots_.size();
    
    widget->setId(id);
    
    roots_.push_back(node);
    widgetMap_[id] = node;
    
    LOG_D << "Root widget added: " << id;
}

void WidgetTree::addChild(const std::string& parentId, std::shared_ptr<Widget> widget, 
                          const std::string& childId) {
    if (!widget) {
        LOG_E << "Cannot add null widget as child";
        return;
    }
    
    auto parentNode = findNode(parentId);
    if (!parentNode) {
        LOG_E << "Parent widget not found: " << parentId;
        return;
    }
    
    auto childNode = std::make_shared<WidgetNode>();
    childNode->widget = widget;
    childNode->id = childId;
    childNode->parent = parentNode;
    childNode->originalIndex = parentNode->children.size();
    
    widget->setId(childId);
    
    parentNode->children.push_back(childNode);
    widgetMap_[childId] = childNode;
    
    LOG_D << "Child widget added: " << parentId << " -> " << childId;
}

void WidgetTree::addToContainer(const std::string& containerId, 
                                 std::shared_ptr<Widget> widget, 
                                 const std::string& widgetId) {
    if (!widget) {
        LOG_E << "Cannot add null widget to container";
        return;
    }
    
    auto containerNode = findNode(containerId);
    if (!containerNode) {
        LOG_E << "Container widget not found: " << containerId;
        return;
    }
    
    // 检查是否是 Container 类型
    auto container = std::dynamic_pointer_cast<Container>(containerNode->widget);
    if (!container) {
        LOG_E << "Widget is not a Container: " << containerId;
        return;
    }
    
    // 通过 Container 的 Add 方法添加
    container->Add(widget, widgetId);
    
    // 同时在 WidgetTree 中注册
    auto widgetNode = std::make_shared<WidgetNode>();
    widgetNode->widget = widget;
    widgetNode->id = widgetId;
    widgetNode->parent = containerNode;
    widgetNode->originalIndex = containerNode->children.size();
    
    containerNode->children.push_back(widgetNode);
    widgetMap_[widgetId] = widgetNode;
    
    LOG_D << "Widget added to container: " << containerId << " -> " << widgetId;
}

void WidgetTree::removeWidget(const std::string& id) {
    auto it = widgetMap_.find(id);
    if (it == widgetMap_.end()) {
        LOG_W << "Widget not found: " << id;
        return;
    }
    
    auto node = it->second;
    
    // 从父节点移除
    if (auto parent = node->parent.lock()) {
        parent->children.erase(
            std::remove_if(parent->children.begin(), parent->children.end(),
                [&node](const std::shared_ptr<WidgetNode>& n) { return n == node; }),
            parent->children.end()
        );
    } else {
        // 从根节点移除
        std::erase_if(roots_, [&node](const std::shared_ptr<WidgetNode>& n) { return n == node; });
    }
    
    // 递归删除子节点
    for (auto& child : node->children) {
        removeWidget(child->id);
    }
    
    widgetMap_.erase(it);
    LOG_D << "Widget removed: " << id;
}

std::shared_ptr<Widget> WidgetTree::getWidget(const std::string& id) {
    auto it = widgetMap_.find(id);
    if (it != widgetMap_.end()) {
        return it->second->widget;
    }
    return nullptr;
}

std::shared_ptr<Widget> WidgetTree::findWidgetAt(float x, float y) {
    // 从后往前遍历（Z 序高的在前）
    for (auto it = roots_.rbegin(); it != roots_.rend(); ++it) {
        auto widget = findWidgetAtInNode(*it, x, y);
        if (widget) {
            return widget;
        }
    }
    return nullptr;
}

void WidgetTree::updateLayout(int screenWidth, int screenHeight) {
    // TODO: 实现布局计算
    // LOG_D << "Layout updated: " << screenWidth << "x" << screenHeight;
}

void WidgetTree::updateTree(CairoGlRenderer& ctx) {
    // 渲染所有根节点
    for (auto& root : roots_) {
        updateNode(ctx, root);
    }
}

void WidgetTree::markAllDirty() {
    for (auto& root : roots_) {
        markNodeDirty(root);
    }
    LOG_D << "All widgets marked dirty";
}


std::shared_ptr<WidgetNode> WidgetTree::findNode(const std::string& id) {
    auto it = widgetMap_.find(id);
    if (it != widgetMap_.end()) {
        return it->second;
    }
    return nullptr;
}

void WidgetTree::updateNode(CairoGlRenderer& ctx, const std::shared_ptr<WidgetNode>& node) {
    if (!node || !node->widget) {
        return;
    }
    
    auto& widget = node->widget;
    LOG_I << "Updating widget: " << widget->getId() << " (zIndex=" << widget->getZIndex() << ")";
    
    // 更新当前控件
    if (widget->isVisible()) {
        LOG_I << "Updating widget: " << widget->getId() << " (zIndex=" << widget->getZIndex() << ")";
        
        // 更新 Widget 纹理（内部会调用 updateWidgetPosition）
        if (widget->isDirty()) {
            widget->updateTexture(ctx);
        }
        
        // 同步 Z 序到渲染器
        widget->updateZIndex(ctx);
    }
    
    // 递归更新子节点（保持原有顺序）
    for (auto& child : node->children) {
        updateNode(ctx, child);
    }
}

std::shared_ptr<Widget> WidgetTree::findWidgetAtInNode(
    const std::shared_ptr<WidgetNode>& node, 
    float x, float y) {
    
    if (!node || !node->widget) {
        return nullptr;
    }
    
    auto& widget = node->widget;
    if (!widget->isVisible()) {
        return nullptr;
    }
    
    // 1. 先递归检查所有子控件（无论父控件是否在范围内）
    for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) {
        auto childWidget = findWidgetAtInNode(*it, x, y);
        if (childWidget) {
            return childWidget;  // 子控件被点击，直接返回
        }
    }
    
    // 2. 再检查父控件本身
    auto pos = widget->getPosition();
    auto size = widget->getSize();
    
    if (x >= pos.x && x <= pos.x + size.width &&
        y >= pos.y && y <= pos.y + size.height) {
        return widget;  // 父控件被点击
    }
    
    return nullptr;
}

void WidgetTree::markNodeDirty(const std::shared_ptr<WidgetNode>& node) {
    if (!node || !node->widget) {
        return;
    }
    
    // 标记当前控件为脏
    node->widget->setVisible(true);  // 触发更新
    
    // 递归标记子节点
    for (auto& child : node->children) {
        markNodeDirty(child);
    }
}

} // namespace Component
