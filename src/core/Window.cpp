#include "Window.hpp"
#include "Logger.hpp"
#include <cstring>
#include <cerrno>
#include <thread>
#include <chrono>

namespace Component {

// Wayland 全局注册回调
void registryGlobal(void* data, wl_registry* registry,
                    uint32_t name, const char* interface, uint32_t version) {
    Window* window = static_cast<Window*>(data);
    
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        window->compositor_ = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        window->wmBase_ = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 2));
    }
}

static void registryGlobalRemove(void* data, wl_registry* registry, uint32_t name) {
    // 暂时不处理
}

static const wl_registry_listener registryListener = {
    .global = registryGlobal,
    .global_remove = registryGlobalRemove
};

Window::Window() = default;

Window::~Window() {
    cleanup();
}

bool Window::create(const std::string& title, int width, int height) {
    width_ = width;
    height_ = height;
    
    // 连接 Wayland 显示器
    display_ = wl_display_connect(nullptr);
    if (!display_) {
        LOG_E << "Failed to connect to Wayland display";
        return false;
    }
    
    // 获取 registry 并绑定 compositor 和 wm_base
    wl_registry* registry = wl_display_get_registry(display_);
    wl_registry_add_listener(registry, &registryListener, this);
    
    // 轮询以获取 compositor 和 wm_base
    wl_display_dispatch(display_);
    wl_display_roundtrip(display_);
    
    if (!compositor_ || !wmBase_) {
        LOG_E << "Failed to get Wayland compositor or wm_base";
        cleanup();
        return false;
    }
    
    // 创建 Surface
    if (!surface_.create(compositor_, wmBase_)) {
        LOG_E << "Failed to create Wayland surface";
        cleanup();
        return false;
    }
    
    // 设置窗口标题
    surface_.setTitle(title);
    
    // ⚠️ 关键修复：等待 xdg_surface configure 事件
    // Wayland 要求在创建 EGL surface 前必须收到 configure 事件
    LOG_I << "Waiting for Wayland surface configuration...";
    
    // 先 commit 一次以触发 configure 事件
    surface_.commit();
    wl_display_flush(display_);
    
    // 等待 configure 事件（最多 2 秒）
    int waitCount = 0;
    const int maxWait = 200; // 200 * 10ms = 2秒
    while (!surface_.isConfigured() && waitCount < maxWait) {
        // 使用 dispatch 阻塞等待事件
        int ret = wl_display_dispatch(display_);
        if (ret < 0) {
            LOG_E << "Wayland display dispatch error: " << strerror(errno);
            break;
        }
        waitCount++;
        
        if (waitCount % 50 == 0) {
            LOG_D << "Still waiting for configure... (" << waitCount << "/" << maxWait << ")";
        }
    }
    
    if (!surface_.isConfigured()) {
        LOG_W << "Surface not configured after " << waitCount << " attempts (" << waitCount * 10 << "ms)";
        LOG_W << "This may cause EGL surface creation to fail";
    } else {
        LOG_I << "Wayland surface configured after " << waitCount << " dispatches (" << waitCount * 10 << "ms)";
    }
    
    // 初始化 EGL
    if (!eglContext_.init(display_)) {
        LOG_E << "Failed to initialize EGL";
        cleanup();
        return false;
    }
    
    // 创建 EGL 表面
    // ⚠️ 关键修复：使用 wl_egl_window 而不是 wl_surface
    eglSurface_ = eglContext_.createSurface(surface_.getEglWindow(), width, height);
    if (eglSurface_ == EGL_NO_SURFACE) {
        LOG_E << "Failed to create EGL surface";
        cleanup();
        return false;
    }
    
    // 激活 EGL 上下文
    if (!eglContext_.makeCurrent(eglSurface_)) {
        LOG_E << "Failed to make EGL context current";
        cleanup();
        return false;
    }
    
    LOG_I << "Window created: " << width << "x" << height;
    return true;
}

void Window::show() {
    if (!visible_) {
        // 配置并显示窗口
        surface_.configure(width_, height_);
        surface_.commit();
        wl_display_flush(display_);
        visible_ = true;
        LOG_I << "Window shown: " << width_ << "x" << height_;
    }
}

void Window::hide() {
    if (visible_) {
        visible_ = false;
        LOG_I << "Window hidden";
    }
}

EGLDisplay Window::getEglDisplay() const {
    return eglContext_.getDisplay();
}

EGLSurface Window::getEglSurface() const {
    return eglSurface_;
}

wl_surface* Window::getWaylandSurface() const {
    return surface_.getSurface();
}

void Window::dispatchEvents() {
    if (display_) {
        wl_display_dispatch_pending(display_);
    }
}

void Window::cleanup() {
    if (eglSurface_ != EGL_NO_SURFACE) {
        eglDestroySurface(eglContext_.getDisplay(), eglSurface_);
        eglSurface_ = EGL_NO_SURFACE;
    }
    
    eglContext_.cleanup();
    surface_.cleanup();
    
    if (wmBase_) {
        xdg_wm_base_destroy(wmBase_);
        wmBase_ = nullptr;
    }
    
    if (compositor_) {
        wl_compositor_destroy(compositor_);
        compositor_ = nullptr;
    }
    
    if (display_) {
        wl_display_disconnect(display_);
        display_ = nullptr;
    }
    
    visible_ = false;
    LOG_D << "Window cleaned up";
}

} // namespace Component
