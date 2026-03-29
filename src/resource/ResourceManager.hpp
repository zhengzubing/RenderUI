#pragma once

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>
#include <cairo/cairo.h>

namespace Component {

/**
 * @brief 资源信息
 */
struct ResourceInfo {
    std::string path;
    cairo_surface_t* surface = nullptr;
    size_t memorySize = 0;
    std::chrono::steady_clock::time_point lastAccessTime;
    int accessCount = 0;
};

/**
 * @brief 资源管理器
 * 
 * 管理图片资源的加载、缓存和清理
 */
class ResourceManager {
public:
    static ResourceManager& instance();
    
    /**
     * @brief 初始化资源管理器
     * @param resourceDir 资源目录
     */
    bool init(const std::string& resourceDir);
    
    /**
     * @brief 加载图片
     * @param path 图片路径（相对于资源目录）
     * @return Cairo 表面，失败返回 nullptr
     */
    cairo_surface_t* loadImage(const std::string& path);
    
    /**
     * @brief 获取图片资源
     * @param path 图片路径
     * @return 资源信息，不存在返回 nullptr
     */
    ResourceInfo* getResource(const std::string& path);
    
    /**
     * @brief 卸载图片
     * @param path 图片路径
     */
    void unloadImage(const std::string& path);
    
    /**
     * @brief 清理未使用的资源
     * @param ttlSeconds 生存时间（秒）
     */
    void cleanupUnused(int ttlSeconds = 300);
    
    /**
     * @brief 获取资源数量
     */
    size_t getResourceCount() const;
    
    /**
     * @brief 获取内存占用（字节）
     */
    size_t getMemoryUsage() const;
    
    /**
     * @brief 清空所有资源
     */
    void clearAll();
    
    /**
     * @brief 获取资源目录
     */
    const std::string& getResourceDir() const { return resourceDir_; }
    
private:
    ResourceManager() = default;
    ~ResourceManager();
    
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    /**
     * @brief 从文件加载图片
     */
    cairo_surface_t* loadFromFile(const std::string& fullPath);
    
    std::string resourceDir_;
    std::map<std::string, std::unique_ptr<ResourceInfo>> resources_;
    mutable std::mutex mutex_;
};

} // namespace Component
