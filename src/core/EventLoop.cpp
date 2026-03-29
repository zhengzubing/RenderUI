#include "EventLoop.hpp"
#include "Logger.hpp"
#include <chrono>
#include <map>

namespace Component {

EventLoop& EventLoop::instance() {
    static EventLoop instance;
    return instance;
}

bool EventLoop::init() {
    if (initialized_) {
        return true;
    }
    
    // 初始化事件处理映射
    handlers_.clear();
    
    initialized_ = true;
    LOG_INFO("EventLoop initialized");
    return true;
}

void EventLoop::run() {
    if (!initialized_) {
        LOG_ERROR("EventLoop not initialized");
        return;
    }
    
    running_ = true;
    LOG_INFO("EventLoop started");
    
    while (running_) {
        // 处理事件队列中的事件
        std::unique_ptr<Event> event;
        
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (!eventQueue_.empty()) {
                event = std::move(eventQueue_.front());
                eventQueue_.pop();
            }
        }
        
        if (event) {
            processEvent(*event);
        } else {
            // 没有事件时短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    LOG_INFO("EventLoop stopped");
}

void EventLoop::quit() {
    running_ = false;
    LOG_DEBUG("EventLoop quit requested");
}

void EventLoop::postEvent(std::unique_ptr<Event> event) {
    if (!event) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        eventQueue_.push(std::move(event));
    }
}

void EventLoop::addHandler(EventType type, EventHandler handler) {
    handlers_[type] = std::move(handler);
    LOG_DEBUG("Event handler added for type: %d", static_cast<int>(type));
}

void EventLoop::processEvent(const Event& event) {
    auto it = handlers_.find(event.type);
    if (it != handlers_.end() && it->second) {
        it->second(event);
    }
}

void EventLoop::cleanup() {
    running_ = false;
    
    // 清空事件队列
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        while (!eventQueue_.empty()) {
            eventQueue_.pop();
        }
    }
    
    handlers_.clear();
    initialized_ = false;
    LOG_DEBUG("EventLoop cleaned up");
}

} // namespace Component
