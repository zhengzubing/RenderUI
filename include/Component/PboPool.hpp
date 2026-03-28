#pragma once

#include <GLES2/gl2.h>
#include <vector>
#include <memory>
#include <mutex>

namespace Component {

/**
 * @brief PBO（Pixel Buffer Object）缓冲池
 * 
 * 用于异步纹理上传，实现 CPU-GPU 并行
 * 使用双缓冲或三缓冲策略避免阻塞
 */
class PboPool {
public:
    /**
     * @brief PBO 配置
     */
    struct Config {
        size_t bufferSize = 0;      // 单个缓冲区大小（字节）
        int bufferCount = 3;        // 缓冲区数量（双缓冲/三缓冲）
        GLenum usage = GL_STREAM_DRAW;  // 使用模式
    };
    
    explicit PboPool(const Config& config = Config());
    ~PboPool();
    
    /**
     * @brief 初始化 PBO 池
     */
    bool init();
    
    /**
     * @brief 获取下一个可用的 PBO 索引
     * @return PBO 索引，失败返回 -1
     */
    int acquireBuffer();
    
    /**
     * @brief 映射 PBO 到 CPU 内存
     * @param index PBO 索引
     * @return CPU 内存指针
     */
    void* mapBuffer(int index);
    
    /**
     * @brief 取消映射 PBO
     * @param index PBO 索引
     */
    void unmapBuffer(int index);
    
    /**
     * @brief 释放 PBO 回池中
     * @param index PBO 索引
     */
    void releaseBuffer(int index);
    
    /**
     * @brief 将 PBO 数据上传到纹理
     * @param index PBO 索引
     * @param textureId OpenGL 纹理 ID
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param format 像素格式
     * @param type 数据类型
     */
    void uploadToTexture(int index, GLuint textureId, 
                         int width, int height,
                         GLenum format, GLenum type);
    
    /**
     * @brief 获取 PBO ID
     * @param index PBO 索引
     * @return OpenGL PBO ID
     */
    GLuint getPboId(int index) const;
    
    /**
     * @brief 获取缓冲区大小
     */
    size_t getBufferSize() const { return config_.bufferSize; }
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    Config config_;
    std::vector<GLuint> pboIds_;
    std::vector<bool> inUse_;
    std::vector<void*> mappedPointers_;
    int nextIndex_ = 0;
    
    mutable std::mutex mutex_;
    bool initialized_ = false;
};

} // namespace Component
