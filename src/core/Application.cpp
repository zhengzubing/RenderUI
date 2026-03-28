#include "Component/Application.hpp"
#include "Component/Logger.hpp"
#include "Component/WindowManager.hpp"
#include "Component/EventLoop.hpp"
#include "Component/ApplicationContext.hpp"
#include "Component/RenderContext.hpp"
#include "Component/ResourceManager.hpp"
#include "Component/DebugMenu.hpp"
#include "Component/EventDispatcher.hpp"
#include <thread>
#include <chrono>

namespace Component {

struct Application::Impl {
    bool running = false;
    std::string title;
    int width = 0;
    int height = 0;
    
    Window* window = nullptr;
    RenderContext renderContext;
};

std::shared_ptr<Application> Application::instance() {
    static auto instance = std::make_shared<Application>();
    return instance;
}

bool Application::init(const std::string& title, int width, int height) {
    impl_ = std::make_unique<Impl>();
    impl_->title = title;
    impl_->width = width;
    impl_->height = height;
    
    // 初始化日志系统
    Logger::instance().init(plog::info, "renderui.log");
    LOG_INFO("Application initializing: %s %dx%d", title.c_str(), width, height);
    
    // 初始化应用上下文
    ApplicationContext::instance().init();
    
    // 初始化资源管理器
    ResourceManager::instance().init("./assets");
    
    // 初始化调试菜单
    DebugMenu::instance().init();
    
    // 初始化事件循环
    EventLoop::instance().init();
    
    // 初始化事件分发器
    EventDispatcher::instance().init();
    
    // 创建主窗口
    impl_->window = WindowManager::instance().createMainWindow(title, width, height);
    if (!impl_->window) {
        LOG_ERROR("Failed to create main window");
        return false;
    }
    
    // 初始化渲染上下文
    if (!impl_->renderContext.init(impl_->window->getEglDisplay(), impl_->window->getEglSurface())) {
        LOG_ERROR("Failed to initialize render context");
        return false;
    }
    
    // 显示窗口
    impl_->window->show();
    
    LOG_INFO("Application initialized successfully");
    return true;
}

int Application::run() {
    if (!impl_ || !impl_->window) {
        LOG_ERROR("Application not initialized");
        return -1;
    }
    
    impl_->running = true;
    LOG_INFO("Application main loop started");
    
    // 主循环
    auto lastFrameTime = std::chrono::steady_clock::now();
    const auto frameDuration = std::chrono::milliseconds(16); // ~60 FPS
    
    while (impl_->running) {
        auto frameStart = std::chrono::steady_clock::now();
        
        // 处理 Wayland 事件
        impl_->window->dispatchEvents();
        
        // 开始渲染
        impl_->renderContext.beginFrame();
        
        // 渲染调试信息（FPS 等）
        DebugMenu::instance().render();
        
        // 结束渲染（交换缓冲区）
        impl_->renderContext.endFrame();
        
        // 更新 FPS
        ApplicationContext::instance().updateFps();
        
        // 控制帧率
        auto frameEnd = std::chrono::steady_clock::now();
        auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
        
        if (frameTime < frameDuration) {
            std::this_thread::sleep_for(frameDuration - frameTime);
        }
    }
    
    LOG_INFO("Application main loop ended");
    return 0;
}

void Application::quit() {
    impl_->running = false;
    LOG_INFO("Application quit requested");
}

void Application::shutdown() {
    LOG_INFO("Application shutting down");
    
    impl_->renderContext.cleanup();
    WindowManager::instance().cleanup();
    EventLoop::instance().cleanup();
    ApplicationContext::instance().cleanup();
    
    impl_.reset();
    
    LOG_INFO("Application shutdown complete");
}

} // namespace Component
