#include "PboPool.hpp"
#include "Logger.hpp"

namespace Component {

PboPool::PboPool(const Config& config) : config_(config) {
    pboIds_.resize(config.bufferCount, 0);
    inUse_.resize(config.bufferCount, false);
    mappedPointers_.resize(config.bufferCount, nullptr);
}

PboPool::~PboPool() {
    cleanup();
}

bool PboPool::init() {
    if (initialized_) {
        return true;
    }
    
    // 生成 PBO
    glGenBuffers(static_cast<GLsizei>(config_.bufferCount), pboIds_.data());
    
    // 初始化每个 PBO
    for (int i = 0; i < config_.bufferCount; i++) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds_[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, 
                     config_.bufferSize,
                     nullptr,  // 不初始化数据
                     config_.usage);
        
        // 检查是否成功
        if (glGetError() != GL_NO_ERROR) {
            LOG_ERROR << "Failed to create PBO " << i;
            cleanup();
            return false;
        }
    }
    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    
    initialized_ = true;
    LOG_INFO << "PboPool initialized: " << config_.bufferCount << " buffers x " << config_.bufferSize << " bytes";
    
    return true;
}

int PboPool::acquireBuffer() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 简单轮询查找可用缓冲区
    for (int i = 0; i < config_.bufferCount; i++) {
        int index = (nextIndex_ + i) % config_.bufferCount;
        if (!inUse_[index]) {
            inUse_[index] = true;
            nextIndex_ = (index + 1) % config_.bufferCount;
            return index;
        }
    }
    
    // 所有缓冲区都在使用中，返回下一个（强制回收）
    int index = nextIndex_;
    inUse_[index] = true;
    nextIndex_ = (index + 1) % config_.bufferCount;
    
    LOG_WARNING << "PboPool exhausted, recycling buffer " << index;
    return index;
}

void* PboPool::mapBuffer(int index) {
    if (index < 0 || index >= config_.bufferCount) {
        return nullptr;
    }
    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds_[index]);
    
    // 映射缓冲区到 CPU 内存
    void* ptr = glMapBufferOES(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY_OES);
    
    if (!ptr) {
        LOG_ERROR << "Failed to map PBO " << index;
        return nullptr;
    }
    
    mappedPointers_[index] = ptr;
    return ptr;
}

void PboPool::unmapBuffer(int index) {
    if (index < 0 || index >= config_.bufferCount) {
        return;
    }
    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds_[index]);
    glUnmapBufferOES(GL_PIXEL_UNPACK_BUFFER);
    
    mappedPointers_[index] = nullptr;
}

void PboPool::releaseBuffer(int index) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (index >= 0 && index < config_.bufferCount) {
        inUse_[index] = false;
        
        // 确保已取消映射
        if (mappedPointers_[index]) {
            unmapBuffer(index);
        }
    }
}

void PboPool::uploadToTexture(int index, GLuint textureId,
                               int width, int height,
                               GLenum format, GLenum type) {
    if (index < 0 || index >= config_.bufferCount) {
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds_[index]);
    
    // 异步上传：OpenGL 会在 GPU 空闲时处理
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    format, type, 0);  // offset 为 0，表示从 PBO 开头
    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

GLuint PboPool::getPboId(int index) const {
    if (index >= 0 && index < static_cast<int>(pboIds_.size())) {
        return pboIds_[index];
    }
    return 0;
}

void PboPool::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 取消所有映射
    for (int i = 0; i < config_.bufferCount; i++) {
        if (mappedPointers_[i]) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds_[i]);
            glUnmapBufferOES(GL_PIXEL_UNPACK_BUFFER);
            mappedPointers_[i] = nullptr;
        }
    }
    
    // 删除 PBO
    if (!pboIds_.empty()) {
        glDeleteBuffers(static_cast<GLsizei>(pboIds_.size()), pboIds_.data());
        pboIds_.assign(config_.bufferCount, 0);
    }
    
    inUse_.assign(config_.bufferCount, false);
    initialized_ = false;
    
    LOG_DEBUG << "PboPool cleaned up";
}

} // namespace Component
