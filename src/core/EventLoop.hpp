#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <atomic>
#include <map>
#include <memory>
#include <cstdint>
#include <thread>

namespace Component {

/**
 * @brief 事件类型
 */
enum class EventType {
    None,
    KeyDown,
    KeyUp,
    TouchDown,
    TouchUp,
    TouchMove,
    TouchCancel,
    WindowResize,
    WindowClose,
    Custom
};

/**
 * @brief 事件基类
 */
struct Event {
    EventType type = EventType::None;
    uint32_t timestamp = 0;
    
    virtual ~Event() = default;
};

/**
 * @brief 触摸事件
 */
struct TouchEvent : public Event {
    float x = 0.0f;
    float y = 0.0f;
    int32_t touchId = 0;
    
    TouchEvent() { type = EventType::TouchDown; }
};

/**
 * @brief 事件循环
 * 
 * 负责事件收集、派发和处理
 */
class EventLoop {
public:
    using EventHandler = std::function<void(const Event&)>;
    
    static EventLoop& instance();
    
    /**
     * @brief 初始化事件循环
     */
    bool init();
    
    /**
     * @brief 运行事件循环
     */
    void run();
    
    /**
     * @brief 请求退出事件循环
     */
    void quit();
    
    /**
     * @brief 推送事件
     */
    void postEvent(std::unique_ptr<Event> event);
    
    /**
     * @brief 注册事件处理器
     */
    void addHandler(EventType type, EventHandler handler);
    
    /**
     * @brief 是否正在运行
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    EventLoop() = default;
    ~EventLoop() = default;
    
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
    
    void processEvent(const Event& event);
    
    std::queue<std::unique_ptr<Event>> eventQueue_;
    std::mutex queueMutex_;
    
    std::map<EventType, EventHandler> handlers_;
    
    std::atomic<bool> running_{false};
    bool initialized_ = false;
};

} // namespace Component
