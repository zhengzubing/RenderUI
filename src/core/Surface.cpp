#include "Surface.hpp"
#include "Logger.hpp"

namespace Component {

// XDG surface 配置回调
static void xdgSurfaceConfigure(void* data, xdg_surface* xdgSurface, uint32_t serial) {
    Surface* surface = static_cast<Surface*>(data);

    // 【第一步：必须 ack】
    xdg_surface_ack_configure(xdgSurface, serial);

    // 标记已配置
    surface->markAsConfigured();

    int width = surface->getWidth();
    int height = surface->getHeight();

    if (!surface->getEglWindow()) {
        surface->createEglWindow(width, height);
        LOG_I << "wl_egl_window created size: " << width << "x" << height;
    } else {
        // 窗口缩放时：动态调整 EGL 窗口大小
        surface->resizeEglWindow(width, height);
        LOG_I << "wl_egl_window resized: " << width << "x" << height;
    }

    // 触发渲染（重绘）
    surface->render();
}

static const xdg_surface_listener xdgSurfaceListener = {
    .configure = xdgSurfaceConfigure
};

// Toplevel 事件回调
static void xdgToplevelConfigure(
    void* data,
    xdg_toplevel* /*toplevel*/,
    int32_t width,
    int32_t height,
    wl_array* /*states*/
) {
    Surface* surface = static_cast<Surface*>(data);
    
    // 保存合成器要求的窗口大小
    if (width > 0 && height > 0) {
        surface->setWindowSize(width, height);
    }
}

// 必须定义的空实现（协议要求）
static void xdgToplevelClose(void* /*data*/, xdg_toplevel* /*toplevel*/) {}

static const struct xdg_toplevel_listener xdgToplevelListener = {
    .configure = xdgToplevelConfigure,
    .close = xdgToplevelClose,
};

Surface::Surface() = default;

Surface::~Surface() {
    cleanup();
}

bool Surface::create(wl_compositor* compositor, xdg_wm_base* wmBase) {
    if (!compositor || !wmBase) {
        LOG_E << "Invalid compositor or wmBase";
        return false;
    }
    
    // 创建 Wayland surface
    surface_ = wl_compositor_create_surface(compositor);
    if (!surface_) {
        LOG_E << "Failed to create wl_surface";
        return false;
    }
    
    // 创建 XDG surface
    xdgSurface_ = xdg_wm_base_get_xdg_surface(wmBase, surface_);
    if (!xdgSurface_) {
        LOG_E << "Failed to create xdg_surface";
        cleanup();
        return false;
    }
    
    // 添加监听器
    xdg_surface_add_listener(xdgSurface_, &xdgSurfaceListener, this);
    
    // 创建 XDG toplevel (窗口)
    toplevel_ = xdg_surface_get_toplevel(xdgSurface_);
    if (!toplevel_) {
        LOG_E << "Failed to create xdg_toplevel";
        cleanup();
        return false;
    }
    
    // 添加 toplevel 监听器
    xdg_toplevel_add_listener(toplevel_, &xdgToplevelListener, this);
    
    // 提交 surface
    wl_surface_commit(surface_);
    // 注意：wl_display_flush 需要 wl_display*，这里简化处理
    
    LOG_I << "Wayland surface created";
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

void Surface::markAsConfigured() {
    configured_ = true;
    LOG_D << "Surface marked as configured";
}

void Surface::createEglWindow(int width, int height) {
    if (eglWindow_) {
        LOG_W << "EGL window already exists";
        return;
    }
    
    if (!surface_) {
        LOG_E << "Cannot create EGL window: surface is null";
        return;
    }
    
    eglWindow_ = wl_egl_window_create(surface_, width, height);
    if (!eglWindow_) {
        LOG_E << "Failed to create wl_egl_window";
        return;
    }
    
    LOG_I << "wl_egl_window created: " << width << "x" << height;
}

void Surface::resizeEglWindow(int width, int height) {
    if (eglWindow_) {
        wl_egl_window_resize(eglWindow_, width, height, 0, 0);
        LOG_D << "wl_egl_window resized to: " << width << "x" << height;
    
        // TODO glViewport
    }
}

void Surface::cleanup() {
    if (eglWindow_) {
        wl_egl_window_destroy(eglWindow_);
        eglWindow_ = nullptr;
        LOG_D << "wl_egl_window destroyed";
    }
    
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
    LOG_D << "Wayland surface cleaned up";
}

void Surface::setWindowSize(int width, int height) {
    width_ = width;
    height_ = height;
    LOG_D << "Window size set to: " << width << "x" << height;
}

void Surface::render() {
    // TODO: 触发渲染逻辑
    // 这里需要通知 Application 或 RenderContext 进行重绘
    LOG_D << "Render triggered";
}

} // namespace Component
