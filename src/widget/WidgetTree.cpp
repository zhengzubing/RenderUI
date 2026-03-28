#include "Component/WidgetTree.hpp"
#include "Component/Logger.hpp"
#include <algorithm>

namespace Component {

struct WidgetTree::Impl {
    int screenWidth = 1920;
    int screenHeight = 1080;
};

WidgetTree::WidgetTree() : impl_(std::make_unique<Impl>()) {}

WidgetTree::~WidgetTree() = default;

void WidgetTree::addRoot(std::shared_ptr<Widget> widget, const std::string& id) {
    if (!widget) {
        LOG_ERROR("Cannot add null widget as root");
        return;
    }
    
    auto node = std::make_shared<WidgetNode>();
    node->widget = widget;
    node->id = id;
    node->zIndex = 0;
    
    widget->setId(id);
    
    roots_.push_back(node);
    widgetMap_[id] = node;
    
    LOG_DEBUG("Root widget added: %s", id.c_str());
}

void WidgetTree::addChild(const std::string& parentId, std::shared_ptr<Widget> widget, 
                          const std::string& childId) {
    if (!widget) {
        LOG_ERROR("Cannot add null widget as child");
        return;
    }
    
    auto parentNode = findNode(parentId);
    if (!parentNode) {
        LOG_ERROR("Parent widget not found: %s", parentId.c_str());
        return;
    }
    
    auto childNode = std::make_shared<WidgetNode>();
    childNode->widget = widget;
    childNode->id = childId;
    childNode->parent = parentNode;
    childNode->zIndex = parentNode->children.size();
    
    widget->setId(childId);
    
    parentNode->children.push_back(childNode);
    widgetMap_[childId] = childNode;
    
    LOG_DEBUG("Child widget added: %s -> %s", parentId.c_str(), childId.c_str());
}

void WidgetTree::removeWidget(const std::string& id) {
    auto it = widgetMap_.find(id);
    if (it == widgetMap_.end()) {
        LOG_WARNING("Widget not found: %s", id.c_str());
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
        roots_.erase(
            std::remove_if(roots_.begin(), roots_.end(),
                [&node](const std::shared_ptr<WidgetNode>& n) { return n == node; }),
            roots_.end()
        );
    }
    
    // 递归删除子节点
    for (auto& child : node->children) {
        removeWidget(child->id);
    }
    
    widgetMap_.erase(it);
    LOG_DEBUG("Widget removed: %s", id.c_str());
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
    impl_->screenWidth = screenWidth;
    impl_->screenHeight = screenHeight;
    
    // TODO: 实现布局计算
    LOG_DEBUG("Layout updated: %dx%d", screenWidth, screenHeight);
}

void WidgetTree::render(RenderContext& ctx) {
    // 按 Z 序排序
    std::vector<std::shared_ptr<WidgetNode>> sortedRoots = roots_;
    std::sort(sortedRoots.begin(), sortedRoots.end(),
        [](const auto& a, const auto& b) { return a->zIndex < b->zIndex; });
    
    // 渲染所有根节点
    for (auto& root : sortedRoots) {
        renderNode(ctx, root);
    }
}

void WidgetTree::markAllDirty() {
    for (auto& root : roots_) {
        markNodeDirty(root);
    }
    LOG_DEBUG("All widgets marked dirty");
}

std::shared_ptr<WidgetNode> WidgetTree::findNode(const std::string& id) {
    auto it = widgetMap_.find(id);
    if (it != widgetMap_.end()) {
        return it->second;
    }
    return nullptr;
}

bool WidgetTree::renderNode(RenderContext& ctx, const std::shared_ptr<WidgetNode>& node) {
    if (!node || !node->widget) {
        return false;
    }
    
    auto& widget = node->widget;
    
    // 渲染当前控件
    if (widget->isVisible() && widget->needsRender()) {
        widget->render(ctx);
    }
    
    // 递归渲染子节点
    for (auto& child : node->children) {
        renderNode(ctx, child);
    }
    
    return true;
}

std::shared_ptr<Widget> WidgetTree::findWidgetAtInNode(const std::shared_ptr<WidgetNode>& node, 
                                                        float x, float y) {
    if (!node || !node->widget) {
        return nullptr;
    }
    
    auto& widget = node->widget;
    if (!widget->isVisible()) {
        return nullptr;
    }
    
    // 检查坐标是否在控件内
    auto pos = widget->getPosition();
    auto size = widget->getSize();
    
    if (x >= pos.x && x <= pos.x + size.width &&
        y >= pos.y && y <= pos.y + size.height) {
        
        // 先检查子节点
        for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) {
            auto childWidget = findWidgetAtInNode(*it, x, y);
            if (childWidget) {
                return childWidget;
            }
        }
        
        // 返回当前控件
        return widget;
    }
    
    return nullptr;
}

void WidgetTree::markNodeDirty(const std::shared_ptr<WidgetNode>& node) {
    if (!node || !node->widget) {
        return;
    }
    
    node->widget->markDirty();
    
    for (auto& child : node->children) {
        markNodeDirty(child);
    }
}

} // namespace Component
