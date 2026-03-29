#include "PerfettoTracker.hpp"
#include "core/Application.hpp"
#include "core/Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace RenderUI;

/**
 * @brief 演示如何在 Application 主循环中集成 Perfetto 性能分析
 * 
 * 使用方法:
 * 1. 编译时启用 -DENABLE_PERFETTO=ON
 * 2. 运行程序，执行操作
 * 3. 按 Ctrl+C 或调用 exportTrace() 导出追踪数据
 * 4. 使用 perfetto UI 查看结果：https://ui.perfetto.dev
 */

class DemoApplication {
public:
    bool init() {
        // 初始化日志
        Logger::init(plog::debug, "renderui_demo.log");
        LOGI("=== DemoApplication Starting ===");
        
        // 初始化 Perfetto 追踪器
        auto& tracker = PerfettoTracker::instance();
        if (!tracker.init(64, 30000)) {  // 64MB, 30 秒
            LOGE("Failed to initialize Perfetto");
            return false;
        }
        
        // 启动追踪
        tracker.startTracing();
        LOGI("Perfetto tracing enabled");
        
        // 初始化 Application
        app_ = std::make_unique<Application>();
        if (!app_->init("RenderUI Demo", 1920, 1080)) {
            LOGE("Failed to initialize Application");
            return false;
        }
        
        return true;
    }
    
    void run() {
        LOGI("Entering main loop...");
        
        int frameCount = 0;
        auto startTime = std::chrono::steady_clock::now();
        
        while (running_) {
            // 使用 RAII 追踪作用域记录帧渲染时间
            PERFETTO_SCOPE("MainFrame");
            
            // 处理事件
            {
                PERFETTO_SCOPE("ProcessEvents");
                app_->processEvents();
            }
            
            // 更新逻辑
            {
                PERFETTO_SCOPE("UpdateLogic");
                updateLogic();
            }
            
            // 渲染场景
            {
                PERFETTO_SCOPE("RenderScene");
                app_->render();
            }
            
            frameCount++;
            
            // 每 60 帧报告一次 FPS
            if (frameCount % 60 == 0) {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    currentTime - startTime).count();
                
                if (elapsed > 0) {
                    double fps = (frameCount * 1000.0) / elapsed;
                    PERFETTO_COUNTER("FPS", static_cast<int64_t>(fps));
                    LOGD("FPS: %.2f", fps);
                }
            }
            
            // 限制帧率到 60 FPS
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        // 导出追踪数据
        exportTrace();
    }
    
    void stop() {
        running_ = false;
    }
    
    void exportTrace() {
        auto& tracker = PerfettoTracker::instance();
        tracker.stopTracing();
        
        std::string traceFile = "demo_trace.trace";
        if (tracker.exportTrace(traceFile)) {
            std::cout << "\n========================================\n";
            std::cout << "Perfetto trace exported to: " << traceFile << "\n";
            std::cout << "View at: https://ui.perfetto.dev\n";
            std::cout << "========================================\n";
        }
    }
    
private:
    void updateLogic() {
        // 模拟一些逻辑更新工作
        // 实际应用中会更新 Widget 状态、动画等
        static float deltaTime = 0.0f;
        deltaTime += 0.016f;
        
        // 模拟 CAN 信号更新
        simulateCanSignals();
    }
    
    void simulateCanSignals() {
        PERFETTO_SCOPE("SimulateCanSignals");
        
        // 模拟车速信号更新
        static float speed = 0.0f;
        speed = 50.0f + std::sin(std::chrono::system_clock::now().time_since_epoch().count() * 0.001) * 10.0f;
        
        // 在实际应用中，这里会通过 DBus 读取真实的 CAN 信号
        // CanSignalManager::instance().update();
    }
    
    std::unique_ptr<Application> app_;
    bool running_ = true;
};

int main(int argc, char* argv[]) {
    std::cout << "RenderUI Perfetto Demo\n";
    std::cout << "======================\n\n";
    std::cout << "This demo demonstrates:\n";
    std::cout << "1. Frame rendering time tracking\n";
    std::cout << "2. Event processing profiling\n";
    std::cout << "3. Logic update timing\n";
    std::cout << "4. FPS counter tracking\n";
    std::cout << "5. Async operation tracing\n\n";
    std::cout << "Press Ctrl+C to stop and export trace data.\n\n";
    
    DemoApplication app;
    if (!app.init()) {
        std::cerr << "Failed to initialize application\n";
        return -1;
    }
    
    // 设置信号处理器以便优雅退出
    std::signal(SIGINT, [](int) {
        std::cout << "\nInterrupted, exporting trace...\n";
    });
    
    app.run();
    
    return 0;
}
