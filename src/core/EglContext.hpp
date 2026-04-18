#pragma once

#include <EGL/egl.h>
#include <string>

namespace Component {

/**
 * @brief EGL 上下文管理
 * 
 * 负责创建和管理 OpenGL ES 上下文
 */
class EglContext {
public:
    EglContext();
    ~EglContext();
    
    /**
     * @brief 初始化 EGL
     * @param display 原生显示连接
     * @return 是否成功
     */
    bool init(void* display);
    
    /**
     * @brief 创建 EGL 表面
     * @param nativeWindow 原生窗口
     * @param width 宽度
     * @param height 高度
     * @return EGLSurface
     */
    EGLSurface createSurface(void* nativeWindow, int width, int height);
    
    /**
     * @brief 激活 EGL 上下文
     * @param surface EGL 表面
     * @return 是否成功
     */
    bool makeCurrent(EGLSurface surface);
    
    /**
     * @brief 交换缓冲区
     * @param surface EGL 表面
     */
    void swapBuffers(EGLSurface surface);
    
    /**
     * @brief 获取 EGL 显示连接
     */
    EGLDisplay getDisplay() const { return display_; }
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLConfig config_ = nullptr;
    EGLContext context_ = EGL_NO_CONTEXT;
    bool initialized_ = false;
};

} // namespace Component
