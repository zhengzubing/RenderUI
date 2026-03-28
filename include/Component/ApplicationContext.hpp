#pragma once

#include <string>
#include <map>
#include <memory>
#include <mutex>

namespace Component {

/**
 * @brief 应用上下文
 * 
 * 全局单例，管理应用级资源和状态
 */
class ApplicationContext {
public:
    static ApplicationContext& instance();
    
    /**
     * @brief 初始化上下文
     */
    bool init();
    
    /**
     * @brief 获取应用数据目录
     */
    const std::string& getDataDir() const { return dataDir_; }
    
    /**
     * @brief 获取资源目录
     */
    const std::string& getResourceDir() const { return resourceDir_; }
    
    /**
     * @brief 设置资源目录
     */
    void setResourceDir(const std::string& dir);
    
    /**
     * @brief 获取帧率
     */
    float getFps() const { return fps_; }
    
    /**
     * @brief 更新帧率
     */
    void updateFps();
    
    /**
     * @brief 获取运行时间 (毫秒)
     */
    uint64_t getRunTimeMs() const;
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    ApplicationContext() = default;
    ~ApplicationContext() = default;
    
    ApplicationContext(const ApplicationContext&) = delete;
    ApplicationContext& operator=(const ApplicationContext&) = delete;
    
    std::string dataDir_;
    std::string resourceDir_;
    
    float fps_ = 0.0f;
    uint64_t startTime_ = 0;
    
    int frameCount_ = 0;
    uint64_t lastFpsTime_ = 0;
    
    mutable std::mutex mutex_;
};

} // namespace Component
