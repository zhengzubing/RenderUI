#include "Component/Application.hpp"

namespace Component {

struct Application::Impl {
    bool running = false;
    std::string title;
    int width = 0;
    int height = 0;
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
    
    // TODO: 初始化窗口系统、EGL 上下文等
    
    return true;
}

int Application::run() {
    impl_->running = true;
    
    // TODO: 实现主循环
    
    return 0;
}

void Application::quit() {
    impl_->running = false;
}

void Application::shutdown() {
    // TODO: 清理资源
    impl_.reset();
}

} // namespace Component
