#include "ResourceManager.hpp"
#include "Logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Component {

ResourceManager& ResourceManager::instance() {
    static ResourceManager instance;
    return instance;
}

ResourceManager::~ResourceManager() {
    clearAll();
}

bool ResourceManager::init(const std::string& resourceDir) {
    std::lock_guard<std::mutex> lock(mutex_);
    resourceDir_ = resourceDir;
    LOG_I << "ResourceManager initialized: " << resourceDir;
    return true;
}

cairo_surface_t* ResourceManager::loadImage(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查是否已缓存
    auto it = resources_.find(path);
    if (it != resources_.end()) {
        it->second->lastAccessTime = std::chrono::steady_clock::now();
        it->second->accessCount++;
        LOG_D << "Image cache hit: " << path;
        return it->second->surface;
    }
    
    // 构建完整路径
    std::string fullPath = resourceDir_ + "/" + path;
    
    // 加载图片
    cairo_surface_t* surface = loadFromFile(fullPath);
    if (!surface) {
        LOG_E << "Failed to load image: " << path;
        return nullptr;
    }
    
    // 创建资源信息
    auto info = std::make_unique<ResourceInfo>();
    info->path = path;
    info->surface = surface;
    info->memorySize = cairo_image_surface_get_stride(surface) * 
                       cairo_image_surface_get_height(surface);
    info->lastAccessTime = std::chrono::steady_clock::now();
    info->accessCount = 1;
    
    LOG_I << "Image loaded: " << path << " (" << cairo_image_surface_get_width(surface) << "x" << cairo_image_surface_get_height(surface) << ", " << info->memorySize / 1024 << " KB)";
    
    cairo_surface_t* result = info->surface;
    resources_[path] = std::move(info);
    
    return result;
}

ResourceInfo* ResourceManager::getResource(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = resources_.find(path);
    if (it != resources_.end()) {
        it->second->lastAccessTime = std::chrono::steady_clock::now();
        it->second->accessCount++;
        return it->second.get();
    }
    
    return nullptr;
}

void ResourceManager::unloadImage(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = resources_.find(path);
    if (it != resources_.end()) {
        if (it->second->surface) {
            cairo_surface_destroy(it->second->surface);
        }
        resources_.erase(it);
        LOG_D << "Image unloaded: " << path;
    }
}

void ResourceManager::cleanupUnused(int ttlSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto ttl = std::chrono::seconds(ttlSeconds);
    
    std::vector<std::string> toRemove;
    
    for (auto& [path, info] : resources_) {
        auto elapsed = now - info->lastAccessTime;
        if (elapsed > ttl) {
            toRemove.push_back(path);
        }
    }
    
    for (const auto& path : toRemove) {
        unloadImage(path);
        LOG_I << "Cleaned up unused resource: " << path;
    }
    
    if (!toRemove.empty()) {
        LOG_I << "Cleaned up " << toRemove.size() << " unused resources";
    }
}

size_t ResourceManager::getResourceCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return resources_.size();
}

size_t ResourceManager::getMemoryUsage() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total = 0;
    for (const auto& [path, info] : resources_) {
        total += info->memorySize;
    }
    return total;
}

void ResourceManager::clearAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& [path, info] : resources_) {
        if (info->surface) {
            cairo_surface_destroy(info->surface);
        }
    }
    resources_.clear();
    
    LOG_I << "All resources cleared";
}

cairo_surface_t* ResourceManager::loadFromFile(const std::string& fullPath) {
    // 使用 stb_image 加载图片
    int width = 0, height = 0, channels = 0;
    
    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &channels, 4);
    if (!data) {
        LOG_E << "stb_image failed to load: " << fullPath;
        return nullptr;
    }
    
    // 创建 Cairo 表面 (ARGB32 格式)
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (!surface) {
        stbi_image_free(data);
        LOG_E << "Failed to create Cairo surface: " << width << "x" << height;
        return nullptr;
    }
    
    // 复制数据到 Cairo 表面
    unsigned char* cairoData = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    
    for (int y = 0; y < height; y++) {
        unsigned char* srcRow = data + y * width * 4;
        unsigned char* dstRow = cairoData + y * stride;
        
        // stb_image 加载的是 RGBA，Cairo 需要 BGRA（小端）
        for (int x = 0; x < width; x++) {
            dstRow[x * 4 + 0] = srcRow[x * 4 + 2];  // B
            dstRow[x * 4 + 1] = srcRow[x * 4 + 1];  // G
            dstRow[x * 4 + 2] = srcRow[x * 4 + 0];  // R
            dstRow[x * 4 + 3] = srcRow[x * 4 + 3];  // A
        }
    }
    
    stbi_image_free(data);
    cairo_surface_mark_dirty(surface);
    
    return surface;
}

} // namespace Component
