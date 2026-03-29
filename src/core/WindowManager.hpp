#pragma once

#include "Component/Window.hpp"
#include <memory>
#include <vector>
#include <string>

namespace Component {

/**
 * @brief 窗口管理器
 * 
 * 管理多个窗口的创建和生命周期
 */
class WindowManager {
public:
    static WindowManager& instance();
    
    /**
     * @brief 创建主窗口
     * @param title 窗口标题
     * @param width 宽度
     * @param height 高度
     * @return 窗口指针
     */
    Window* createMainWindow(const std::string& title, int width, int height);
    
    /**
     * @brief 获取主窗口
     */
    Window* getMainWindow() const { return mainWindow_.get(); }
    
    /**
     * @brief 销毁窗口
     */
    void destroyWindow(Window* window);
    
    /**
     * @brief 清理所有窗口
     */
    void cleanup();
    
private:
    WindowManager() = default;
    ~WindowManager() = default;
    
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    
    std::unique_ptr<Window> mainWindow_;
    std::vector<std::unique_ptr<Window>> windows_;
};

} // namespace Component
