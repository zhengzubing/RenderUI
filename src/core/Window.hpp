#pragma once

#include "Types.hpp"
#include "Surface.hpp"
#include "EglContext.hpp"
#include <string>
#include <memory>

namespace Component {

/**
 * @brief 窗口对象
 * 
 * 封装 Wayland 窗口和 EGL 上下文
 */
class Window {
public:
    Window();
    ~Window();
    
    /**
     * @brief 创建窗口
     * @param title 窗口标题
     * @param width 宽度
     * @param height 高度
     * @return 是否成功
     */
    bool create(const std::string& title, int width, int height);
    
    /**
     * @brief 显示窗口
     */
    void show();
    
    /**
     * @brief 隐藏窗口
     */
    void hide();
    
    /**
     * @brief 获取宽度
     */
    int getWidth() const { return width_; }
    
    /**
     * @brief 获取高度
     */
    int getHeight() const { return height_; }
    
    /**
     * @brief 获取 EGL 显示连接
     */
    EGLDisplay getEglDisplay() const;
    
    /**
     * @brief 获取 EGL 表面
     */
    EGLSurface getEglSurface() const;
    
    /**
     * @brief 获取 Wayland display
     */
    wl_display* getWaylandDisplay() const { return display_; }
    
    /**
     * @brief 获取 Wayland surface
     */
    wl_surface* getWaylandSurface() const;
    
    /**
     * @brief 轮询 Wayland 事件
     */
    void dispatchEvents();
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    wl_display* display_ = nullptr;
    wl_compositor* compositor_ = nullptr;
    xdg_wm_base* wmBase_ = nullptr;
    
    // 友元函数，用于 Wayland 回调
    friend void registryGlobal(void* data, wl_registry* registry,
                               uint32_t name, const char* interface, uint32_t version);
    
    Surface surface_;
    EglContext eglContext_;
    
    EGLSurface eglSurface_ = EGL_NO_SURFACE;
    
    int width_ = 0;
    int height_ = 0;
    bool visible_ = false;
};

} // namespace Component
