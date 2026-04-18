#pragma once

#include <string>
#include <map>
#include <functional>
#include <mutex>

namespace Component {

/**
 * @brief 调试菜单项
 */
struct DebugMenuItem {
    std::string label;
    std::string description;
    std::function<void()> callback;
    bool enabled = true;
};

/**
 * @brief 键盘调试菜单
 * 
 * 通过键盘快捷键控制调试功能
 * 
 * 快捷键:
 * - F1: 打开/关闭调试菜单
 * - 1-9: 触发对应菜单项
 * - F2: 切换 FPS 显示
 * - F3: 切换绘制区域显示
 * - F4: 切换内存信息显示
 * - F5: 重载配置
 * - F6: 切换主题
 * - F12: 截图
 */
class DebugMenu {
public:
    static DebugMenu& instance();
    
    /**
     * @brief 初始化调试菜单
     */
    bool init();
    
    /**
     * @brief 添加菜单项
     * @param key 快捷键 (0-9)
     * @param label 标签
     * @param description 描述
     * @param callback 回调函数
     */
    void addItem(int key, const std::string& label, 
                 const std::string& description,
                 std::function<void()> callback);
    
    /**
     * @brief 移除菜单项
     * @param key 快捷键
     */
    void removeItem(int key);
    
    /**
     * @brief 处理键盘事件
     * @param keyCode 键码
     * @return 是否已处理
     */
    bool handleKey(int keyCode);
    
    /**
     * @brief 是否显示菜单
     */
    bool isVisible() const { return visible_; }
    
    /**
     * @brief 设置菜单可见性
     */
    void setVisible(bool visible);
    
    /**
     * @brief 切换菜单可见性
     */
    void toggle();
    
    /**
     * @brief 渲染调试菜单
     */
    void render();
    
    /**
     * @brief 获取 FPS 显示状态
     */
    bool isFpsVisible() const { return showFps_; }
    
    /**
     * @brief 设置 FPS 显示状态
     */
    void setShowFps(bool show);
    
    /**
     * @brief 获取绘制区域显示状态
     */
    bool isDrawCallsVisible() const { return showDrawCalls_; }
    
    /**
     * @brief 设置绘制区域显示状态
     */
    void setShowDrawCalls(bool show);
    
private:
    DebugMenu() = default;
    ~DebugMenu() = default;
    
    DebugMenu(const DebugMenu&) = delete;
    DebugMenu& operator=(const DebugMenu&) = delete;
    
    void renderOverlay();
    void renderFps();
    void renderMenu();
    
    std::map<int, DebugMenuItem> items_;
    mutable std::mutex mutex_;
    
    bool visible_ = false;
    bool showFps_ = true;
    bool showDrawCalls_ = false;
    bool showMemory_ = false;
    bool initialized_ = false;
};

} // namespace Component
