#include "Surface.hpp"
#include "Logger.hpp"

namespace Component {

// XDG surface 配置回调
static void xdgSurfaceConfigure(void* data, xdg_surface* xdgSurface, uint32_t serial) {
    Surface* surface = static_cast<Surface*>(data);
    xdg_surface_ack_configure(xdgSurface, serial);
    // 标记为已配置
}

static const xdg_surface_listener xdgSurfaceListener = {
    .configure = xdgSurfaceConfigure
};

// Toplevel 事件回调
static void xdgToplevelConfigure(void* data, xdg_toplevel* toplevel,
                                  int32_t width, int32_t height,
                                  wl_array* states) {
    // 可以处理窗口大小变化
}

static void xdgToplevelClose(void* data, xdg_toplevel* toplevel) {
    // 可以处理窗口关闭
}

static const xdg_toplevel_listener xdgToplevelListener = {
    .configure = xdgToplevelConfigure,
    .close = xdgToplevelClose
};

Surface::Surface() = default;

Surface::~Surface() {
    cleanup();
}

bool Surface::create(wl_compositor* compositor, xdg_wm_base* wmBase) {
    if (!compositor || !wmBase) {
        LOG_ERROR << "Invalid compositor or wmBase";
        return false;
    }
    
    // 创建 Wayland surface
    surface_ = wl_compositor_create_surface(compositor);
    if (!surface_) {
        LOG_ERROR << "Failed to create wl_surface";
        return false;
    }
    
    // 创建 XDG surface
    xdgSurface_ = xdg_wm_base_get_xdg_surface(wmBase, surface_);
    if (!xdgSurface_) {
        LOG_ERROR << "Failed to create xdg_surface";
        cleanup();
        return false;
    }
    
    // 添加监听器
    xdg_surface_add_listener(xdgSurface_, &xdgSurfaceListener, this);
    
    // 创建 XDG toplevel (窗口)
    toplevel_ = xdg_surface_get_toplevel(xdgSurface_);
    if (!toplevel_) {
        LOG_ERROR << "Failed to create xdg_toplevel";
        cleanup();
        return false;
    }
    
    // 添加 toplevel 监听器
    xdg_toplevel_add_listener(toplevel_, &xdgToplevelListener, this);
    
    // 提交 surface
    wl_surface_commit(surface_);
    wl_display_flush(wl_surface_get_display(surface_));
    
    LOG_INFO << "Wayland surface created";
    return true;
}

void Surface::setTitle(const std::string& title) {
    if (toplevel_) {
        xdg_toplevel_set_title(toplevel_, title.c_str());
        xdg_toplevel_set_app_id(toplevel_, "com.renderui.app");
    }
}

void Surface::configure(int width, int height) {
    if (toplevel_) {
        // 请求窗口大小（实际大小由合成器决定）
        wl_surface_commit(surface_);
    }
}

void Surface::commit() {
    if (surface_) {
        wl_surface_commit(surface_);
    }
}

void Surface::cleanup() {
    if (toplevel_) {
        xdg_toplevel_destroy(toplevel_);
        toplevel_ = nullptr;
    }
    
    if (xdgSurface_) {
        xdg_surface_destroy(xdgSurface_);
        xdgSurface_ = nullptr;
    }
    
    if (surface_) {
        wl_surface_destroy(surface_);
        surface_ = nullptr;
    }
    
    configured_ = false;
    LOG_DEBUG << "Wayland surface cleaned up";
}

} // namespace Component
