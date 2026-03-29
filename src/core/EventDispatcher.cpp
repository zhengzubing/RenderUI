#include "EventDispatcher.hpp"
#include "Logger.hpp"

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
    
    EventLoop::instance().addHandler(EventType::KeyDown, [this](const Event& event) {
        dispatchKeyEvent(static_cast<const KeyEvent&>(event));
    });
    
    EventLoop::instance().addHandler(EventType::KeyUp, [this](const Event& event) {
        dispatchKeyEvent(static_cast<const KeyEvent&>(event));
    });
    
    LOG_INFO << "EventDispatcher initialized";
    return true;
}

void EventDispatcher::setWidgetTree(WidgetTree* tree) {
    widgetTree_ = tree;
    LOG_DEBUG << "WidgetTree set for event dispatching";
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
        LOG_VERBOSE << "No widget found at (" << event.x << ", " << event.y << ")";
        return false;
    }
    
    // 派发到控件
    bool handled = targetWidget->handleTouchEvent(event);
    
    if (handled) {
        LOG_DEBUG << "Touch event dispatched to widget: " << targetWidget->getId();
    }
    
    return handled;
}

bool EventDispatcher::dispatchKeyEvent(const KeyEvent& event) {
    // 首先通知监听器
    auto it = listeners_.find(event.type);
    if (it != listeners_.end()) {
        for (auto& listener : it->second) {
            if (listener(event)) {
                return true;
            }
        }
    }
    
    // TODO: 键盘事件派发（需要焦点系统）
    LOG_DEBUG << "Key event received: keyCode=" << event.keyCode;
    return false;
}

void EventDispatcher::addListener(EventType type, EventListener listener) {
    listeners_[type].push_back(std::move(listener));
    LOG_DEBUG << "Event listener added for type: " << static_cast<int>(type);
}

} // namespace Component
