#pragma once

#include "PboPool.hpp"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace Component {

/**
 * @brief 异步纹理上传任务
 */
struct UploadTask {
    uint8_t* data = nullptr;      // CPU 数据指针
    size_t dataSize = 0;           // 数据大小
    int width = 0;                 // 纹理宽度
    int height = 0;                // 纹理高度
    GLenum format = GL_RGBA;       // 像素格式
    GLenum type = GL_UNSIGNED_BYTE;// 数据类型
    GLuint textureId = 0;          // 目标纹理 ID
    
    std::function<void(bool)> onComplete;  // 完成回调
};

/**
 * @brief 异步纹理上传器
 * 
 * 使用独立线程和 PBO 池实现异步纹理上传
 * 避免阻塞渲染主线程
 */
class AsyncTextureUploader {
public:
    static AsyncTextureUploader& instance();
    
    /**
     * @brief 初始化上传器
     * @param pboBufferSize 单个 PBO 缓冲区大小
     * @param pboBufferCount PBO 数量
     */
    bool init(size_t pboBufferSize = 1024 * 1024, int pboBufferCount = 3);
    
    /**
     * @brief 提交上传任务
     * @param task 上传任务
     */
    void submit(UploadTask task);
    
    /**
     * @brief 停止上传器
     */
    void stop();
    
    /**
     * @brief 是否正在运行
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief 获取队列长度
     */
    size_t getQueueSize() const;
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    AsyncTextureUploader() = default;
    ~AsyncTextureUploader();
    
    AsyncTextureUploader(const AsyncTextureUploader&) = delete;
    AsyncTextureUploader& operator=(const AsyncTextureUploader&) = delete;
    
    /**
     * @brief 上传工作线程
     */
    void workerThread();
    
    /**
     * @brief 执行单个上传任务
     */
    void executeTask(UploadTask& task);
    
    std::queue<UploadTask> taskQueue_;
    mutable std::mutex queueMutex_;
    std::condition_variable queueCV_;
    
    std::unique_ptr<std::thread> workerThread_;
    std::atomic<bool> running_{false};
    
    std::unique_ptr<PboPool> pboPool_;
};

} // namespace Component
