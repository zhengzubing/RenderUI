#pragma once

#include "Widget.hpp"
#include "RenderContext.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>

namespace Component {

/**
 * @brief 控件树节点
 */
struct WidgetNode {
    std::shared_ptr<Widget> widget;
    std::shared_ptr<WidgetNode> parent;
    std::vector<std::shared_ptr<WidgetNode>> children;
    
    std::string id;
    int zIndex = 0;
};

/**
 * @brief 控件树管理
 * 
 * 管理所有控件的层级结构和生命周期
 */
class WidgetTree {
public:
    WidgetTree();
    ~WidgetTree();
    
    /**
     * @brief 添加根控件
     */
    void addRoot(std::shared_ptr<Widget> widget, const std::string& id);
    
    /**
     * @brief 添加子控件
     */
    void addChild(const std::string& parentId, std::shared_ptr<Widget> widget, const std::string& childId);
    
    /**
     * @brief 移除控件
     */
    void removeWidget(const std::string& id);
    
    /**
     * @brief 获取控件
     */
    std::shared_ptr<Widget> getWidget(const std::string& id);
    
    /**
     * @brief 查找控件 (通过坐标)
     */
    std::shared_ptr<Widget> findWidgetAt(float x, float y);
    
    /**
     * @brief 更新布局
     */
    void updateLayout(int screenWidth, int screenHeight);
    
    /**
     * @brief 渲染所有需要渲染的控件
     */
    void render(RenderContext& ctx);
    
    /**
     * @brief 标记所有控件为脏
     */
    void markAllDirty();
    
    /**
     * @brief 获取根节点
     */
    const std::vector<std::shared_ptr<WidgetNode>>& getRoots() const { return roots_; }
    
private:
    std::vector<std::shared_ptr<WidgetNode>> roots_;
    std::map<std::string, std::shared_ptr<WidgetNode>> widgetMap_;
    
    std::shared_ptr<WidgetNode> findNode(const std::string& id);
    bool renderNode(RenderContext& ctx, const std::shared_ptr<WidgetNode>& node);
};

} // namespace Component
