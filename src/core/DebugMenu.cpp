#include "DebugMenu.hpp"
#include "Logger.hpp"
#include "ApplicationContext.hpp"
#include "ResourceManager.hpp"
#include <sstream>
#include <iomanip>

namespace Component {

DebugMenu& DebugMenu::instance() {
    static DebugMenu instance;
    return instance;
}

bool DebugMenu::init() {
    if (initialized_) {
        return true;
    }
    
    // 添加默认菜单项
    addItem(1, "Toggle FPS", "切换帧率显示", [this]() {
        setShowFps(!showFps_);
        LOG_INFO("FPS display: %s", showFps_ ? "ON" : "OFF");
    });
    
    addItem(2, "Show Draw Calls", "显示绘制调用", [this]() {
        showDrawCalls_ = !showDrawCalls_;
        LOG_INFO("Draw calls display: %s", showDrawCalls_ ? "ON" : "OFF");
    });
    
    addItem(3, "Show Memory", "显示内存使用", [this]() {
        showMemory_ = !showMemory_;
        auto mem = ResourceManager::instance().getMemoryUsage();
        LOG_INFO("Memory display: %s (%zu KB)", showMemory_ ? "ON" : "OFF", mem / 1024);
    });
    
    addItem(4, "Dump Scene Graph", "输出场景图", []() {
        LOG_INFO("=== Scene Graph Dump ===");
        // TODO: 实现场景图输出
        LOG_INFO("========================");
    });
    
    addItem(5, "Reload Config", "重载配置", []() {
        LOG_INFO("Reloading configuration...");
        // TODO: 实现配置重载
    });
    
    addItem(6, "Simulate CAN Signal", "模拟车速信号", []() {
        LOG_INFO("Simulating CAN signal...");
        // TODO: 实现 CAN 信号模拟
    });
    
    addItem(7, "Switch Theme", "切换主题", []() {
        LOG_INFO("Switching theme...");
        // TODO: 实现主题切换
    });
    
    addItem(8, "Language", "切换语言", []() {
        LOG_INFO("Switching language...");
        // TODO: 实现语言切换
    });
    
    addItem(9, "Clean Resources", "清理资源", []() {
        ResourceManager::instance().cleanupUnused(60);
        LOG_INFO("Resources cleaned up");
    });
    
    initialized_ = true;
    LOG_INFO("DebugMenu initialized");
    return true;
}

void DebugMenu::addItem(int key, const std::string& label, 
                        const std::string& description,
                        std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    DebugMenuItem item;
    item.label = label;
    item.description = description;
    item.callback = std::move(callback);
    item.enabled = true;
    
    items_[key] = std::move(item);
    LOG_DEBUG("Debug menu item added: %d - %s", key, label.c_str());
}

void DebugMenu::removeItem(int key) {
    std::lock_guard<std::mutex> lock(mutex_);
    items_.erase(key);
}

bool DebugMenu::handleKey(int keyCode) {
    // F1 切换菜单
    if (keyCode == 0xFFBE) {  // XK_F1
        toggle();
        return true;
    }
    
    // F2 切换 FPS
    if (keyCode == 0xFFBF) {  // XK_F2
        setShowFps(!showFps_);
        return true;
    }
    
    // F3 切换绘制区域
    if (keyCode == 0xFFC0) {  // XK_F3
        showDrawCalls_ = !showDrawCalls_;
        return true;
    }
    
    // F4 切换内存信息
    if (keyCode == 0xFFC1) {  // XK_F4
        showMemory_ = !showMemory_;
        return true;
    }
    
    // F5 重载配置
    if (keyCode == 0xFFC2) {  // XK_F5
        items_[5].callback();
        return true;
    }
    
    // F12 截图
    if (keyCode == 0xFFC9) {  // XK_F12
        LOG_INFO("Screenshot requested (not implemented)");
        return true;
    }
    
    // 数字键 1-9
    if (!visible_) {
        return false;
    }
    
    int key = 0;
    if (keyCode >= '1' && keyCode <= '9') {
        key = keyCode - '0';
    }
    
    if (key > 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = items_.find(key);
        if (it != items_.end() && it->second.enabled && it->second.callback) {
            it->second.callback();
            return true;
        }
    }
    
    // ESC 关闭菜单
    if (keyCode == 0xFF1B) {  // XK_Escape
        setVisible(false);
        return true;
    }
    
    return false;
}

void DebugMenu::setVisible(bool visible) {
    visible_ = visible;
    LOG_DEBUG("Debug menu visibility: %s", visible ? "ON" : "OFF");
}

void DebugMenu::toggle() {
    setVisible(!visible_);
}

void DebugMenu::setShowFps(bool show) {
    showFps_ = show;
}

void DebugMenu::render() {
    if (showFps_) {
        renderFps();
    }
    
    if (visible_) {
        renderMenu();
    }
    
    if (showMemory_) {
        // 渲染内存信息
        auto mem = ResourceManager::instance().getMemoryUsage();
        std::ostringstream oss;
        oss << "Memory: " << (mem / 1024) << " KB";
        // TODO: 实际渲染
    }
}

void DebugMenu::renderFps() {
    auto& ctx = ApplicationContext::instance();
    float fps = ctx.getFps();
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << "FPS: " << fps;
    
    // TODO: 实际渲染到屏幕右上角
    LOG_VERBOSE("%s", oss.str().c_str());
}

void DebugMenu::renderMenu() {
    // TODO: 渲染调试菜单覆盖层
    LOG_DEBUG("Rendering debug menu...");
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    LOG_INFO("=== Debug Menu ===");
    for (const auto& [key, item] : items_) {
        LOG_INFO(" [%d] %s - %s", key, item.label.c_str(), item.description.c_str());
    }
    LOG_INFO("===================");
}

} // namespace Component
