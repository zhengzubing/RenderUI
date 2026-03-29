#include "WindowManager.hpp"
#include "Logger.hpp"

namespace Component {

WindowManager& WindowManager::instance() {
    static WindowManager instance;
    return instance;
}

Window* WindowManager::createMainWindow(const std::string& title, int width, int height) {
    if (mainWindow_) {
        LOG_WARNING << "Main window already created";
        return mainWindow_.get();
    }
    
    mainWindow_ = std::make_unique<Window>();
    if (!mainWindow_->create(title, width, height)) {
        LOG_ERROR << "Failed to create main window";
        mainWindow_.reset();
        return nullptr;
    }
    
    LOG_INFO << "Main window created: " << title << ", " << width << "x" << height;
    return mainWindow_.get();
}

void WindowManager::destroyWindow(Window* window) {
    if (!window) {
        return;
    }
    
    // 查找并移除窗口
    auto it = std::find_if(windows_.begin(), windows_.end(),
        [window](const std::unique_ptr<Window>& w) {
            return w.get() == window;
        });
    
    if (it != windows_.end()) {
        windows_.erase(it);
        LOG_INFO << "Window destroyed";
    }
}

void WindowManager::cleanup() {
    for (auto& window : windows_) {
        window->cleanup();
    }
    windows_.clear();
    
    if (mainWindow_) {
        mainWindow_->cleanup();
        mainWindow_.reset();
    }
    
    LOG_INFO << "WindowManager cleaned up";
}

} // namespace Component
