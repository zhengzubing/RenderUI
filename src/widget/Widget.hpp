#pragma once

#include "Types.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace Component {

// 前向声明
class RenderContext;
class Canvas;
struct TouchEvent;

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
     * @brief 获取 X 坐标
     */
    float getX() const;
    
    /**
     * @brief 设置 X 坐标
     */
    void setX(float x);
    
    /**
     * @brief 获取 Y 坐标
     */
    float getY() const;
    
    /**
     * @brief 设置 Y 坐标
     */
    void setY(float y);
    
    /**
     * @brief 获取大小
     */
    Size getSize() const;
    
    /**
     * @brief 获取宽度
     */
    float getWidth() const;
    
    /**
     * @brief 设置宽度
     */
    void setWidth(float width);
    
    /**
     * @brief 获取高度
     */
    float getHeight() const;
    
    /**
     * @brief 设置高度
     */
    void setHeight(float height);
    
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
    
    // /**
    //  * @brief 添加子控件
    //  */
    // void addChild(std::shared_ptr<Widget> child);
    
    // /**
    //  * @brief 移除子控件
    //  */
    // void removeChild(const std::string& id);
    
    /**
     * @brief 获取父控件
     */
    std::shared_ptr<Widget> getParent() const;
    
    /**
     * @brief 设置 ID
     */
    void setId(const std::string& id);
    
    /**
     * @brief 获取 ID
     */
    const std::string& getId() const;
    
    /**
     * @brief 获取 Z 序
     */
    int getZIndex() const;
    
    /**
     * @brief 设置 Z 序
     */
    void setZIndex(int zIndex);
    
    /**
     * @brief 处理触摸事件
     */
    virtual bool handleTouchEvent(const TouchEvent& event);
    
    /**
     * @brief 从 JSON 配置初始化控件
     * @param config JSON 配置对象
     */
    virtual void fromJson(const json& config);

    /**
     * @brief 标记为脏，需要重绘
     */
    void markDirty();
    
protected:
    /**
     * @brief 子类实现绘制逻辑
     */
    virtual void onDraw(Canvas& canvas);

    std::string id_;
    float x_ = 0.0f;
    float y_ = 0.0f;
    float width_ = 0.0f;
    float height_ = 0.0f;
    bool visible_ = true;
    bool dirty_ = false;
    int zIndex_ = 0;
    
    std::weak_ptr<Widget> parent_;
};

} // namespace Component
