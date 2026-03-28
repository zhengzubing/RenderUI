#include "Component/Application.hpp"
#include "Component/Logger.hpp"

using namespace Component;

int main(int argc, char* argv[]) {
    // 获取应用实例
    auto app = Application::instance();
    
    // 初始化应用（创建 1920x1080 窗口）
    if (!app->init("RenderUI Demo", 1920, 1080)) {
        PLOG_ERROR << "Failed to initialize application";
        return -1;
    }
    
    PLOG_INFO << "Application started successfully";
    
    // 运行主循环
    int result = app->run();
    
    // 清理资源
    app->shutdown();
    
    return result;
}
