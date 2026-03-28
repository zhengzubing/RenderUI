#pragma once

#include <wayland-client.h>
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
     * @brief 提交 surface 变更
     */
    void commit();
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    wl_surface* surface_ = nullptr;
    xdg_surface* xdgSurface_ = nullptr;
    xdg_toplevel* toplevel_ = nullptr;
    bool configured_ = false;
};

} // namespace Component
