#pragma once

#include "RenderContext.hpp"
#include <memory>
#include <string>


namespace Component {

class Window;
/**
 * @brief 应用入口与主循环管理
 * 
 * 负责初始化窗口系统、事件循环和渲染上下文
 */
class Application {
public:
    /**
     * @brief 获取单例实例
     */
    static std::shared_ptr<Application> instance();
    
    /**
     * @brief 初始化应用
     * @param title 窗口标题
     * @param width 窗口宽度
     * @param height 窗口高度
     * @return 是否成功
     */
    bool init(const std::string& title, int width, int height);
    
    /**
     * @brief 运行主循环
     * @return 退出码
     */
    int run();
    
    /**
     * @brief 请求退出应用
     */
    void quit();
    
    /**
     * @brief 清理资源
     */
    void shutdown();
    
private:
    bool running_ = false;
    std::string title_;
    int width_ = 0;
    int height_ = 0;
    
    Window* window_ = nullptr;
    RenderContext renderContext_;
    
public:
    // 允许 shared_ptr 正确销毁对象
    ~Application() = default;
};

} // namespace Component
