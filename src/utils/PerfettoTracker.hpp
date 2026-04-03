#ifndef COMPONENT_PERFETTO_TRACKER_HPP
#define COMPONENT_PERFETTO_TRACKER_HPP

#include <string>
#include <memory>
#include <cstdint>

// 前向声明 Perfetto 类型
namespace perfetto {
    class TracingSession;
}

// 引入 Perfetto SDK 头文件
#if ENABLE_PERFETTO
#include <perfetto.h>
#endif

namespace RenderUI {

/**
 * @brief Perfetto 性能分析追踪器
 * 
 * 集成 Perfetto SDK，提供系统级性能分析能力
 * 支持跟踪渲染帧率、事件处理、资源加载等关键路径
 * 
 * @note 需要链接 perfetto SDK (-lperfetto)
 * @see https://perfetto.dev/docs/instrumentation/tracing-sdk
 */
class PerfettoTracker {
public:
    /**
     * @brief 获取单例实例
     * @return PerfettoTracker& 单例引用
     */
    static PerfettoTracker& instance();
    
    /**
     * @brief 初始化 Perfetto 追踪系统
     * @param maxBufferSizeMB 缓冲区大小 (MB)，默认 64MB
     * @param durationMs 追踪持续时间 (ms)，默认 30000ms (30 秒)
     * @return bool 成功返回 true
     */
    bool init(uint32_t maxBufferSizeMB = 64, uint32_t durationMs = 30000);
    
    /**
     * @brief 启动追踪会话
     * @return bool 成功返回 true
     */
    bool startTracing();
    
    /**
     * @brief 停止追踪会话
     */
    void stopTracing();
    
    /**
     * @brief 导出追踪数据到文件
     * @param filePath 输出文件路径 (.trace 格式)
     * @return bool 成功返回 true
     */
    bool exportTrace(const std::string& filePath);
    
    /**
     * @brief 是否已初始化
     * @return bool 已初始化返回 true
     */
    bool isInitialized() const { return initialized_; }
    
    /**
     * @brief 是否在追踪中
     * @return bool 正在追踪返回 true
     */
    bool isTracing() const { return tracingActive_; }

private:
    // 私有构造函数和拷贝控制，防止误用
    PerfettoTracker() = default;
    ~PerfettoTracker() = default;
    PerfettoTracker(const PerfettoTracker&) = delete;
    PerfettoTracker& operator=(const PerfettoTracker&) = delete;
    
    bool initialized_ = false;
    bool tracingActive_ = false;
    uint32_t maxBufferSizeMB_ = 64;
    uint32_t durationMs_ = 30000;
    
    // Perfetto 资源
    std::unique_ptr<perfetto::TracingSession> tracingSession_;
};

/**
 * @brief RAII 风格的追踪作用域
 * 
 * 自动记录函数或代码块的执行时间
 * 在构造时开始追踪，析构时结束追踪
 * 
 * @example
 * void renderFrame() {
 *     PERFETTO_SCOPE("RenderFrame");
 *     // ... 渲染代码
 * }
 */
class PerfettoScope {
public:
    /**
     * @brief 创建追踪作用域
     * @param name 追踪名称
     */
    explicit PerfettoScope(const char* name);
    ~PerfettoScope();
    
private:
    const char* name_;
};

/**
 * @brief 追踪异步操作的开始
 * @param name 追踪名称
 * @param id 异步操作 ID (用于关联开始和结束)
 */
void perfettoTrackAsyncBegin(const char* name, uint64_t id);

/**
 * @brief 追踪异步操作的结束
 * @param name 追踪名称
 * @param id 异步操作 ID (用于关联开始和结束)
 */
void perfettoTrackAsyncEnd(const char* name, uint64_t id);

/**
 * @brief 记录计数器事件 (如 FPS、内存使用量)
 * @param name 计数器名称
 * @param value 计数值
 */
void perfettoTrackCounter(const char* name, int64_t value);

} // namespace RenderUI

// 便捷的宏定义
#define PERFETTO_SCOPE(name) RenderUI::PerfettoScope _perfetto_scope_##__LINE__(name)
#define PERFETTO_ASYNC_BEGIN(name, id) RenderUI::perfettoTrackAsyncBegin(name, id)
#define PERFETTO_ASYNC_END(name, id) RenderUI::perfettoTrackAsyncEnd(name, id)
#define PERFETTO_COUNTER(name, value) RenderUI::perfettoTrackCounter(name, value)

#endif // COMPONENT_PERFETTO_TRACKER_HPP
