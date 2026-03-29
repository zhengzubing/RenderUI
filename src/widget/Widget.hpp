#pragma once

#include "Types.hpp"
#include "EventLoop.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Component {

// 前向声明
class RenderContext;
class Canvas;

/**
 * @brief 控件基类
 * 
 * 所有 UI 控件的基类，提供位置、大小、可见性等基本属性
 */
class Widget : public std::enable_shared_from_this<Widget> {
public:
    Widget();
    virtual ~Widget();
    
    /**
     * @brief 设置位置
     */
    virtual void setPosition(float x, float y);
    
    /**
     * @brief 设置大小
     */
    virtual void setSize(float width, float height);
    
    /**
     * @brief 设置可见性
     */
    virtual void setVisible(bool visible);
    
    /**
     * @brief 获取位置
     */
    Vec2 getPosition() const;
    
    /**
     * @brief 获取大小
     */
    Size getSize() const;
    
    /**
     * @brief 是否可见
     */
    bool isVisible() const;
    
    /**
     * @brief 是否需要渲染
     */
    virtual bool needsRender() const;
    
    /**
     * @brief 渲染控件
     */
    virtual void render(RenderContext& ctx);
    
    /**
     * @brief 添加子控件
     */
    void addChild(std::shared_ptr<Widget> child);
    
    /**
     * @brief 移除子控件
     */
    void removeChild(const std::string& id);
    
    /**
     * @brief 获取子控件
     */
    const std::vector<std::shared_ptr<Widget>>& getChildren() const { return children_; }
    
    /**
     * @brief 获取父控件
     */
    std::shared_ptr<Widget> getParent() const;
    
    /**
     * @brief 设置 ID
     */
    void setId(const std::string& id) { id_ = id; }
    
    /**
     * @brief 获取 ID
     */
    const std::string& getId() const { return id_; }
    
    /**
     * @brief 设置 Z 序
     */
    void setZIndex(int zIndex) { zIndex_ = zIndex; }
    
    /**
     * @brief 获取 Z 序
     */
    int getZIndex() const { return zIndex_; }
    
    /**
     * @brief 处理触摸事件
     */
    virtual bool handleTouchEvent(const TouchEvent& event);
    
    /**
     * @brief 处理键盘事件
     */
    virtual bool handleKeyEvent(const KeyEvent& event);
    
protected:
    /**
     * @brief 子类实现绘制逻辑
     */
    virtual void onDraw(Canvas& canvas);
    
    /**
     * @brief 标记为脏，需要重绘
     */
    void markDirty();
    
    std::string id_;
    Vec2 position_;
    Size size_;
    bool visible_ = true;
    bool dirty_ = false;
    int zIndex_ = 0;
    
    std::vector<std::shared_ptr<Widget>> children_;
    std::weak_ptr<Widget> parent_;
    
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Component
