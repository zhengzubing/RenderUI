#pragma once

#include "Widget.hpp"
#include <vector>
#include <map>
#include <string>
#include <memory>

namespace Component {

/**
 * @brief 容器控件
 * 
 * 可包含子控件的容器，支持 Add/Remove 方式管理子控件
 */
class Container : public Widget {
public:
    Container() = default;
    ~Container() override = default;
    
    /**
     * @brief 添加子控件
     * @param widget 要添加的控件
     * @param id 控件 ID（可选，为空则使用 widget 的 ID）
     */
    void Add(std::shared_ptr<Widget> widget, const std::string& id = "");
    
    /**
     * @brief 添加子控件（便捷版本，返回自身支持链式调用）
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> AddChild(Args&&... args) {
        auto child = std::make_shared<T>(std::forward<Args>(args)...);
        Add(child);
        return child;
    }
    
    /**
     * @brief 移除子控件
     * @param id 控件 ID
     */
    void Remove(const std::string& id);
    
    /**
     * @brief 获取子控件
     * @param id 控件 ID
     * @return 子控件，不存在返回 nullptr
     */
    std::shared_ptr<Widget> Get(const std::string& id) const;
    
    /**
     * @brief 获取所有子控件
     */
    const std::vector<std::shared_ptr<Widget>>& GetChildren() const { return children_; }
    
    /**
     * @brief 渲染容器及所有子控件
     */
    void render(RenderContext& ctx) override;
    
    /**
     * @brief 处理触摸事件
     */
    bool handleTouchEvent(const TouchEvent& event) override;
    
    /**
     * @brief 从 JSON 配置初始化
     */
    void fromJson(const json& config) override {
        // 先调用基类处理公共属性
        Widget::fromJson(config);
    }
    
protected:
    std::vector<std::shared_ptr<Widget>> children_;
    std::map<std::string, std::shared_ptr<Widget>> childMap_;
};

} // namespace Component
