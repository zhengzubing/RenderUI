#pragma once

#include "EventLoop.hpp"
#include "WidgetTree.hpp"
#include <memory>

namespace Component {

/**
 * @brief 事件分发器
 * 
 * 负责将事件派发到正确的控件
 */
class EventDispatcher {
public:
    static EventDispatcher& instance();
    
    /**
     * @brief 初始化事件分发器
     */
    bool init();
    
    /**
     * @brief 设置控件树
     */
    void setWidgetTree(WidgetTree* tree);
    
    /**
     * @brief 分发触摸事件
     * @param event 触摸事件
     * @return 是否已处理
     */
    bool dispatchTouchEvent(const TouchEvent& event);
    
    /**
     * @brief 分发键盘事件
     * @param event 键盘事件
     * @return 是否已处理
     */
    bool dispatchKeyEvent(const KeyEvent& event);
    
    /**
     * @brief 注册事件监听器
     */
    using EventListener = std::function<bool(const Event&)>;
    void addListener(EventType type, EventListener listener);
    
private:
    EventDispatcher() = default;
    ~EventDispatcher() = default;
    
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;
    
    WidgetTree* widgetTree_ = nullptr;
    std::map<EventType, std::vector<EventListener>> listeners_;
};

} // namespace Component
