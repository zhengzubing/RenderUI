#include "EventDispatcher.hpp"
#include "Logger.hpp"
#include "EventLoop.hpp"

namespace Component {

EventDispatcher& EventDispatcher::instance() {
    static EventDispatcher instance;
    return instance;
}

bool EventDispatcher::init() {
    // 注册到事件循环
    EventLoop::instance().addHandler(EventType::TouchDown, [this](const Event& event) {
        dispatchTouchEvent(static_cast<const TouchEvent&>(event));
    });
    
    EventLoop::instance().addHandler(EventType::TouchUp, [this](const Event& event) {
        dispatchTouchEvent(static_cast<const TouchEvent&>(event));
    });
    
    EventLoop::instance().addHandler(EventType::TouchMove, [this](const Event& event) {
        dispatchTouchEvent(static_cast<const TouchEvent&>(event));
    });
    
    EventLoop::instance().addHandler(EventType::TouchCancel, [this](const Event& event) {
        dispatchTouchEvent(static_cast<const TouchEvent&>(event));
    });
    
    LOG_I << "EventDispatcher initialized";
    return true;
}

void EventDispatcher::setWidgetTree(WidgetTree* tree) {
    widgetTree_ = tree;
    LOG_D << "WidgetTree set for event dispatching";
}

bool EventDispatcher::dispatchTouchEvent(const TouchEvent& event) {
    // 首先通知监听器
    auto it = listeners_.find(event.type);
    if (it != listeners_.end()) {
        for (auto& listener : it->second) {
            if (listener(event)) {
                return true;  // 已处理
            }
        }
    }
    
    // 然后通过控件树查找目标控件
    if (!widgetTree_) {
        return false;
    }
    
    // 从控件树中找到命中的控件
    auto targetWidget = widgetTree_->findWidgetAt(event.x, event.y);
    if (!targetWidget) {
        LOG_V << "No widget found at (" << event.x << ", " << event.y << ")";
        return false;
    }
    
    // 派发到控件
    bool handled = targetWidget->handleTouchEvent(event);
    
    if (handled) {
        LOG_D << "Touch event dispatched to widget: " << targetWidget->getId();
    }
    
    return handled;
}

void EventDispatcher::addListener(EventType type, EventListener listener) {
    listeners_[type].push_back(std::move(listener));
    LOG_D << "Event listener added for type: " << static_cast<int>(type);
}

} // namespace Component
