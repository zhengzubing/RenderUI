#include "Application.hpp"
#include "WidgetTree.hpp"
#include "Logger.hpp"
#include "WindowManager.hpp"
#include "EventLoop.hpp"
#include "ApplicationContext.hpp"
#include "RenderContext.hpp"
#include "ResourceManager.hpp"
#include "DebugMenu.hpp"
#include "EventDispatcher.hpp"
#include <thread>
#include <chrono>

namespace Component {

std::shared_ptr<Application> Application::instance() {
    static auto instance = std::make_shared<Application>();
    return instance;
}

bool Application::init(const std::string& title, int width, int height) {
    title_ = title;
    width_ = width;
    height_ = height;
    
    // 初始化日志系统
    Logger::instance().init(plog::debug, "renderui.log");
    
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
    
    // 创建控件树
    widgetTree_ = std::make_unique<WidgetTree>();
    
    // 创建主窗口
    window_ = WindowManager::instance().createMainWindow(title, width, height);
    if (!window_) {
        LOG_E << "Failed to create main window";
        return false;
    }
    
    // 初始化渲染上下文
    if (!renderContext_.init(window_->getEglDisplay(), window_->getEglSurface())) {
        LOG_E << "Failed to initialize render context";
        return false;
    }
    
    // 显示窗口
    window_->show();
    
    LOG_I << "Application initialized successfully";
    return true;
}

int Application::run() {
    if (!window_) {
        LOG_E << "Application not initialized";
        return -1;
    }
    
    running_ = true;
    LOG_I << "Application main loop started";
    
    // 主循环
    const auto frameDuration = std::chrono::milliseconds(16); // ~60 FPS
    
    while (running_) {
        auto frameStart = std::chrono::steady_clock::now();
        
        // 处理 Wayland 事件
        window_->dispatchEvents();
        
        // 更新布局（在渲染前）
        if (widgetTree_) {
            widgetTree_->updateLayout(width_, height_);
        }
        
        // 开始渲染
        renderContext_.beginFrame();
        
        // 自动渲染控件树（类似 Qt 的自动渲染）
        if (widgetTree_) {
            widgetTree_->render(renderContext_);
        } else {
            LOG_W << "Widget tree is null!";
        }
        
        // 渲染调试信息（FPS 等）
        DebugMenu::instance().render();
        
        // 结束渲染（交换缓冲区）
        renderContext_.endFrame();
        
        // 更新 FPS
        ApplicationContext::instance().updateFps();
        
        // 控制帧率
        auto frameEnd = std::chrono::steady_clock::now();
        auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
        
        if (frameTime < frameDuration) {
            std::this_thread::sleep_for(frameDuration - frameTime);
        }
    }
    
    LOG_I << "Application main loop ended";
    return 0;
}

void Application::quit() {
    running_ = false;
    LOG_I << "Application quit requested";
}

void Application::shutdown() {
    LOG_I << "Application shutting down";
    
    widgetTree_.reset();
    renderContext_.cleanup();
    WindowManager::instance().cleanup();
    EventLoop::instance().cleanup();
    ApplicationContext::instance().cleanup();
    
    LOG_I << "Application shutdown complete";
}

WidgetTree& Application::getWidgetTree() {
    if (!widgetTree_) {
        widgetTree_ = std::make_unique<WidgetTree>();
    }
    return *widgetTree_;
}

} // namespace Component
