#pragma once

#include <wayland-client.h>
#include <wayland-egl.h>
#include <xdg-shell-client-protocol.h>
#include <string>

namespace Component {

/**
 * @brief Wayland 表面封装
 * 
 * 管理 Wayland surface 和 xdg_toplevel
 */
class Surface {
public:
    Surface();
    ~Surface();
    
    /**
     * @brief 创建 Wayland surface
     * @param compositor Wayland compositor
     * @param wmBase XDG WM base
     * @return 是否成功
     */
    bool create(wl_compositor* compositor, xdg_wm_base* wmBase);
    
    /**
     * @brief 配置窗口标题
     */
    void setTitle(const std::string& title);
    
    /**
     * @brief 配置窗口大小
     */
    void configure(int width, int height);
    
    /**
     * @brief 获取 Wayland surface
     */
    wl_surface* getSurface() const { return surface_; }
    
    /**
     * @brief 获取 xdg surface
     */
    xdg_surface* getXdgSurface() const { return xdgSurface_; }
    
    /**
     * @brief 获取 xdg toplevel
     */
    xdg_toplevel* getToplevel() const { return toplevel_; }
    
    /**
     * @brief 获取 Wayland EGL window（用于 EGL surface 创建）
     */
    wl_egl_window* getEglWindow() const { return eglWindow_; }
    
    /**
     * @brief 检查是否已配置
     */
    bool isConfigured() const { return configured_; }
    
    /**
     * @brief 提交 surface 变更
     */
    void commit();
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
    /**
     * @brief 标记为已配置（内部使用）
     */
    void markAsConfigured();
    
    /**
     * @brief 创建 EGL window（内部使用）
     */
    void createEglWindow(int width, int height);
    
    /**
     * @brief 调整 EGL window 大小
     */
    void resizeEglWindow(int width, int height);
    
    /**
     * @brief 设置窗口尺寸（内部使用）
     */
    void setWindowSize(int width, int height);
    
    /**
     * @brief 获取窗口宽度
     */
    int getWidth() const { return width_; }
    
    /**
     * @brief 获取窗口高度
     */
    int getHeight() const { return height_; }
    
    /**
     * @brief 触发渲染
     */
    void render();
    
private:
    wl_surface* surface_ = nullptr;
    wl_egl_window* eglWindow_ = nullptr;
    xdg_surface* xdgSurface_ = nullptr;
    xdg_toplevel* toplevel_ = nullptr;
    bool configured_ = false;
    int width_ = 1280;   // 默认宽度
    int height_ = 960;  // 默认高度
};

} // namespace Component
