#include "Component/AsyncTextureUploader.hpp"
#include "Component/Logger.hpp"
#include <cstring>

namespace Component {

AsyncTextureUploader& AsyncTextureUploader::instance() {
    static AsyncTextureUploader instance;
    return instance;
}

AsyncTextureUploader::~AsyncTextureUploader() {
    stop();
}

bool AsyncTextureUploader::init(size_t pboBufferSize, int pboBufferCount) {
    if (running_) {
        return true;
    }
    
    // 创建 PBO 池
    PboPool::Config config;
    config.bufferSize = pboBufferSize;
    config.bufferCount = pboBufferCount;
    config.usage = GL_STREAM_DRAW;
    
    pboPool_ = std::make_unique<PboPool>(config);
    
    if (!pboPool_->init()) {
        LOG_ERROR("Failed to initialize PboPool");
        return false;
    }
    
    // 启动工作线程
    running_ = true;
    workerThread_ = std::make_unique<std::thread>(&AsyncTextureUploader::workerThread, this);
    
    LOG_INFO("AsyncTextureUploader initialized: %zu bytes x %d buffers", 
             pboBufferSize, pboBufferCount);
    
    return true;
}

void AsyncTextureUploader::submit(UploadTask task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        taskQueue_.push(std::move(task));
    }
    
    queueCV_.notify_one();
}

void AsyncTextureUploader::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    queueCV_.notify_all();
    
    if (workerThread_) {
        workerThread_->join();
        workerThread_.reset();
    }
    
    cleanup();
    
    LOG_INFO("AsyncTextureUploader stopped");
}

size_t AsyncTextureUploader::getQueueSize() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return taskQueue_.size();
}

void AsyncTextureUploader::workerThread() {
    LOG_INFO("AsyncTextureUploader worker thread started");
    
    while (running_) {
        UploadTask task;
        
        // 等待任务
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] {
                return !taskQueue_.empty() || !running_;
            });
            
            if (!running_ && taskQueue_.empty()) {
                break;
            }
            
            if (!taskQueue_.empty()) {
                task = std::move(taskQueue_.front());
                taskQueue_.pop();
            }
        }
        
        // 执行上传
        if (task.data && task.textureId != 0) {
            executeTask(task);
        }
    }
    
    LOG_INFO("AsyncTextureUploader worker thread exiting");
}

void AsyncTextureUploader::executeTask(UploadTask& task) {
    // 获取 PBO
    int pboIndex = pboPool_->acquireBuffer();
    if (pboIndex < 0) {
        LOG_ERROR("Failed to acquire PBO buffer");
        if (task.onComplete) {
            task.onComplete(false);
        }
        return;
    }
    
    // 映射 PBO 并复制数据
    void* mappedPtr = pboPool_->mapBuffer(pboIndex);
    if (!mappedPtr) {
        LOG_ERROR("Failed to map PBO");
        pboPool_->releaseBuffer(pboIndex);
        if (task.onComplete) {
            task.onComplete(false);
        }
        return;
    }
    
    // 复制数据到 PBO
    std::memcpy(mappedPtr, task.data, task.dataSize);
    pboPool_->unmapBuffer(pboIndex);
    
    // 上传到纹理（异步）
    pboPool_->uploadToTexture(pboIndex, task.textureId,
                               task.width, task.height,
                               task.format, task.type);
    
    // 释放 PBO
    pboPool_->releaseBuffer(pboIndex);
    
    LOG_DEBUG("Texture uploaded: %dx%d to GL texture %d", 
              task.width, task.height, task.textureId);
    
    // 通知完成
    if (task.onComplete) {
        task.onComplete(true);
    }
}

void AsyncTextureUploader::cleanup() {
    if (pboPool_) {
        pboPool_->cleanup();
    }
    
    // 清空队列
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        while (!taskQueue_.empty()) {
            taskQueue_.pop();
        }
    }
}

} // namespace Component
