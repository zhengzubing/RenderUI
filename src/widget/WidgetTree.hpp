#pragma once

#include "Widget.hpp"
#include "CairoGlRenderer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>

namespace Component {

/**
 * @brief 控件树节点
 */
struct WidgetNode {
    std::shared_ptr<Widget> widget;
    std::weak_ptr<WidgetNode> parent;
    std::vector<std::shared_ptr<WidgetNode>> children;
    
    std::string id;
    int originalIndex = 0;
    
    /**
     * @brief 获取节点的字符串描述
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << "WidgetNode[" << id << "]";
        
        if (widget) {
            oss << " Type=" << widget->getId();
            oss << " Pos=(" << widget->getX() << "," << widget->getY() << ")";
            oss << " Size=" << widget->getWidth() << "x" << widget->getHeight();
            oss << " Visible=" << (widget->isVisible() ? "Y" : "N");
        } else {
            oss << " [NULL WIDGET]";
        }
        
        oss << " Children=" << children.size();
        
        return oss.str();
    }
    
    /**
     * @brief 流输出运算符（友元函数）
     */
    friend std::ostream& operator<<(std::ostream& os, const WidgetNode& node) {
        os << node.toString();
        return os;
    }
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
     * @brief 添加到容器（支持 Container 类型）
     */
    void addToContainer(const std::string& containerId, std::shared_ptr<Widget> widget, const std::string& widgetId);
    
    /**
     * @brief 添加子控件（旧接口，兼容用）
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
    void updateTree(CairoGlRenderer& ctx);
    
    /**
     * @brief 标记所有控件为脏
     */
    void markAllDirty();
    
    /**
     * @brief 获取根节点
     */
    const std::vector<std::shared_ptr<WidgetNode>>& getTopLevelWidgets() const { return roots_; }
    
private:
    std::vector<std::shared_ptr<WidgetNode>> roots_;
    std::map<std::string, std::shared_ptr<WidgetNode>> widgetMap_;
    
    std::shared_ptr<WidgetNode> findNode(const std::string& id);
    void updateNode(CairoGlRenderer& ctx, const std::shared_ptr<WidgetNode>& node);
    std::shared_ptr<Widget> findWidgetAtInNode(const std::shared_ptr<WidgetNode>& node, float x, float y);
    void markNodeDirty(const std::shared_ptr<WidgetNode>& node);
};

} // namespace Component
