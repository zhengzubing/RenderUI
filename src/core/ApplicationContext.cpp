#include "ApplicationContext.hpp"
#include "Logger.hpp"
#include <chrono>
#include <cstdlib>
#include <sys/time.h>

namespace Component {

ApplicationContext& ApplicationContext::instance() {
    static ApplicationContext instance;
    return instance;
}

bool ApplicationContext::init() {
    // 初始化数据目录
    const char* home = getenv("HOME");
    if (home) {
        dataDir_ = std::string(home) + "/.renderui";
    } else {
        dataDir_ = "/tmp/renderui";
    }
    
    // 默认资源目录为当前目录的 assets
    resourceDir_ = "./assets";
    
    // 记录启动时间
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    startTime_ = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    lastFpsTime_ = startTime_;
    
    LOG_INFO << "ApplicationContext initialized, data dir: " << dataDir_;
    return true;
}

void ApplicationContext::setResourceDir(const std::string& dir) {
    resourceDir_ = dir;
    LOG_INFO << "Resource directory set to: " << dir;
}

void ApplicationContext::updateFps() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t currentTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    
    frameCount_++;
    
    // 每秒更新一次 FPS
    if (currentTime - lastFpsTime_ >= 1000) {
        fps_ = static_cast<float>(frameCount_) * 1000.0f / static_cast<float>(currentTime - lastFpsTime_);
        frameCount_ = 0;
        lastFpsTime_ = currentTime;
        
        LOG_VERBOSE << "FPS: " << fps_;
    }
}

uint64_t ApplicationContext::getRunTimeMs() const {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    uint64_t currentTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return currentTime - startTime_;
}

void ApplicationContext::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    fps_ = 0.0f;
    frameCount_ = 0;
    LOG_DEBUG << "ApplicationContext cleaned up";
}

} // namespace Component
